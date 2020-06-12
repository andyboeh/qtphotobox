#include "qtphotobox.h"
#include "statemachine.h"
#include "initwidget.h"
#include "settingswidget.h"
#include "settings.h"
#include "idlewidget.h"
#include "greeterwidget.h"
#include "startwidget.h"
#include "capturewidget.h"
#include "countdownwidget.h"
#include "pbcamera.h"
#include "assemblewidget.h"
#include "reviewwidget.h"
#include "storageManager.h"
#include "pictureWorker.h"
#include "printer.h"
#include "postprocessWorker.h"
#include "postprocesswidget.h"
#include "archivewidget.h"
#include "showWorker.h"
#include "gpioWorker.h"
#include "errorwidget.h"
#include "waitremovablewidget.h"
#include "screensaver.h"
#include "screensaverwidget.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QFontDatabase>
#include <QWindow>
#include <QScreen>
#include <QKeyEvent>
#include <QProcess>

MainWindow::MainWindow()
{
    mCurrentWidget = nullptr;
    mOverlayWidget = nullptr;
    mCameraThread = nullptr;
    mPrinterThread = nullptr;
    mShowThread = nullptr;
    mShowThreadObject = nullptr;
    mPrinterThreadObject = nullptr;
    mPictureWorkerThread = nullptr;
    mPictureWorkerThreadObject = nullptr;
    mPostprocessWorkerThread = nullptr;
    mPostprocessWorkerThreadObject = nullptr;
    mCameraThreadObject = nullptr;
    mGpioThread = nullptr;
    mGpioThreadObject = nullptr;
    mScreenSaver = nullptr;
    mImagesCaptured = 0;
    mImagesToCapture = 0;
    mErrorPresent = false;
}

MainWindow::~MainWindow()
{
    if(mScreenSaver) {
        delete mScreenSaver;
        mScreenSaver = nullptr;
    }
    if(mCameraThread) {
        if(mCameraThread->isRunning()) {
            emit stopCameraThread();
            mCameraThread->quit();
            mCameraThread->wait();
        }
    }
    if(mPictureWorkerThread) {
        if(mPictureWorkerThread->isRunning()) {
            emit stopPictureWorkerThread();
            mPictureWorkerThread->quit();
            mPictureWorkerThread->wait();
        }
    }
    if(mPostprocessWorkerThread) {
        if(mPostprocessWorkerThread->isRunning()) {
            emit stopPostprocessWorkerThread();
            mPostprocessWorkerThread->quit();
            mPostprocessWorkerThread->wait();
        }
    }
    if(mPrinterThread) {
        if(mPrinterThread->isRunning()) {
            emit stopPrinterThread();
            mPrinterThread->quit();
            mPrinterThread->wait();
        }
    }
    if(mShowThread) {
        if(mShowThread->isRunning()) {
            emit stopShowThread();
            mShowThread->quit();
            mShowThread->wait();
        }
    }
    if(mGpioThread) {
        if(mGpioThread->isRunning()) {
            emit stopGpioThread();
            mGpioThread->quit();
            mGpioThread->wait();
        }
    }
    mCurrentWidget->deleteLater();
    mCurrentWidget = nullptr;
}

void MainWindow::changeState(QString name)
{
    qDebug() << "changeState";
    StateMachine &sm = StateMachine::getInstance();
    qDebug() << name;
    if(mErrorPresent)
        return;
    if(mOverlayWidget) {
        delete mOverlayWidget;
        mOverlayWidget = nullptr;
    }
    if(name == "start") {
        delete mCurrentWidget;
        mCurrentWidget = new startWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "restart") {
        qApp->quit();
        QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
    } else if(name == "waitremovable") {
        delete mCurrentWidget;
        mCurrentWidget = new waitRemovableWidget();
        setCentralWidget(mCurrentWidget);
        storageManager &stm = storageManager::getInstance();
        connect(&stm, SIGNAL(removableDeviceDetected(QString)), this, SLOT(removableDeviceDetected(QString)));
        connect(this, SIGNAL(waitForRemovableDevice()), &stm, SLOT(waitForRemovableDevice()));
        emit waitForRemovableDevice();
    } else if(name == "init") {
        delete mCurrentWidget;
        mCurrentWidget = new initWidget();
        setCentralWidget(mCurrentWidget);
        initThreads();
        connect(mCameraThreadObject, SIGNAL(cameraInitialized(bool)), mCurrentWidget, SLOT(cameraInitialized(bool)));
        connect(this, SIGNAL(initializeCamera()), mCameraThreadObject, SLOT(initCamera()));
        emit initializeCamera();
    } else if(name == "settings") {
        delete mCurrentWidget;
        mCurrentWidget = new settingsWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "idle") {
        delete mCurrentWidget;
        mCurrentWidget = new idleWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "screensaver") {
        delete mCurrentWidget;
        mCurrentWidget = new screensaverWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "archive") {
        delete mCurrentWidget;
        mCurrentWidget = new archiveWidget();
        if(mPrinterThreadObject)
            connect(mCurrentWidget, SIGNAL(printAssembledPicture(QString,int)), mPrinterThreadObject, SLOT(addFilePrintJob(QString,int)));
        setCentralWidget(mCurrentWidget);
    } else if(name == "greeter") {
        mImagesCaptured = 0;
        delete mCurrentWidget;
        mCurrentWidget = new greeterWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "countdown") {
        delete mCurrentWidget;
        mCurrentWidget = new countdownWidget(mCameraThreadObject);
        setCentralWidget(mCurrentWidget);
    } else if(name == "capture") {
        mImageToPrint = "";
        mImageToReview = QPixmap();
        delete mCurrentWidget;
        mCurrentWidget = new captureWidget(mCameraThreadObject);
        setCentralWidget(mCurrentWidget);
    } else if(name == "assemble") {
        if(mImagesCaptured < mImagesToCapture) {
            sm.triggerState("countdown");
        } else {
            emit finishTask();
            delete mCurrentWidget;
            mCurrentWidget = new assembleWidget();
            setCentralWidget(mCurrentWidget);
            qDebug() << "All images captured, assembling...";
        }
    } else if(name == "review") {
        delete mCurrentWidget;
        mCurrentWidget = new reviewWidget(mImageToReview);
        setCentralWidget(mCurrentWidget);
    } else if(name == "postprocess") {
        mOverlayWidget = new postprocessWidget(centralWidget());
        connect(mOverlayWidget, SIGNAL(startPrintJob(int)), this, SLOT(startPrintJob(int)));
        mOverlayWidget->show();
    } else if(name == "error") {
        //mOverlayWidget = new errorWidget(centralWidget());
    } else if(name == "teardown") {
        QApplication::quit();
    }
}

void MainWindow::initThreads()
{
    pbSettings &pbs = pbSettings::getInstance();
    if(!mCameraThread) {
        mCameraThread = new QThread();
        mCameraThreadObject = new pbCamera();
        mCameraThreadObject->moveToThread(mCameraThread);
        connect(mCameraThreadObject, SIGNAL(finished()), mCameraThread, SLOT(quit()));
        connect(mCameraThreadObject, SIGNAL(finished()), mCameraThread, SLOT(deleteLater()));
        connect(mCameraThreadObject, SIGNAL(imageCaptured(QPixmap)), this, SLOT(imageCaptured(QPixmap)));
        connect(mCameraThreadObject, SIGNAL(cameraError(QString)), this, SLOT(cameraError(QString)));
        connect(mCameraThread, SIGNAL(started()), mCameraThreadObject, SLOT(start()));
        connect(mCameraThread, SIGNAL(finished()), mCameraThreadObject, SLOT(deleteLater()));
        connect(this, SIGNAL(stopCameraThread()), mCameraThreadObject, SLOT(stop()));
        mCameraThread->start();
    }
    if(!mPictureWorkerThread) {
        mPictureWorkerThread = new QThread();
        mPictureWorkerThreadObject = new pictureWorker();
        mPictureWorkerThreadObject->moveToThread(mPictureWorkerThread);
        connect(mPictureWorkerThreadObject, SIGNAL(finished()), mPictureWorkerThread, SLOT(quit()));
        connect(mPictureWorkerThreadObject, SIGNAL(finished()), mPictureWorkerThread, SLOT(deleteLater()));
        connect(mPictureWorkerThreadObject, SIGNAL(pictureAssembled(QPixmap)), this, SLOT(pictureAssembled(QPixmap)));
        connect(mPictureWorkerThreadObject, SIGNAL(pictureError(QString)), this, SLOT(genericError(QString)));
        connect(mPictureWorkerThread, SIGNAL(started()), mPictureWorkerThreadObject, SLOT(start()));
        connect(mPictureWorkerThread, SIGNAL(finished()), mPictureWorkerThreadObject, SLOT(deleteLater()));
        connect(this, SIGNAL(stopPictureWorkerThread()), mPictureWorkerThreadObject, SLOT(stop()));
        connect(this, SIGNAL(initAssembleTask()), mPictureWorkerThreadObject, SLOT(initAssembleTask()));
        connect(this, SIGNAL(finishTask()), mPictureWorkerThreadObject, SLOT(finishTask()));
        connect(this, SIGNAL(addPicture(QPixmap)), mPictureWorkerThreadObject, SLOT(addPicture(QPixmap)));
        mPictureWorkerThread->start();
    }
    if(!mPostprocessWorkerThread) {
        mPostprocessWorkerThread = new QThread();
        mPostprocessWorkerThreadObject = new postprocessWorker();
        mPostprocessWorkerThreadObject->moveToThread(mPostprocessWorkerThread);
        connect(mPostprocessWorkerThreadObject, SIGNAL(finished()), mPostprocessWorkerThread, SLOT(quit()));
        connect(mPostprocessWorkerThreadObject, SIGNAL(finished()), mPostprocessWorkerThread, SLOT(deleteLater()));
        connect(mPostprocessWorkerThreadObject, SIGNAL(fullImageSaved(QString,QString,bool)), this, SLOT(fullImageSaved(QString,QString,bool)));
        connect(mPostprocessWorkerThreadObject, SIGNAL(assembledImageSaved(QString,QString,bool)), this, SLOT(assembledImageSaved(QString,QString,bool)));
        connect(mPostprocessWorkerThreadObject, SIGNAL(postprocessError(QString)), this, SLOT(genericError(QString)));
        connect(mPostprocessWorkerThread, SIGNAL(started()), mPostprocessWorkerThreadObject, SLOT(start()));
        connect(mPostprocessWorkerThread, SIGNAL(finished()), mPostprocessWorkerThreadObject, SLOT(deleteLater()));
        connect(this, SIGNAL(stopPostprocessWorkerThread()), mPostprocessWorkerThreadObject, SLOT(stop()));
        connect(this, SIGNAL(saveFullPicture(QPixmap)), mPostprocessWorkerThreadObject, SLOT(saveFullImage(QPixmap)));
        connect(this, SIGNAL(saveAssembledPicture(QPixmap)), mPostprocessWorkerThreadObject, SLOT(saveAssembledImage(QPixmap)));
        connect(this, SIGNAL(saveThumbnail(QString)), mPostprocessWorkerThreadObject, SLOT(saveThumbnail(QString)));
        connect(mPostprocessWorkerThreadObject, SIGNAL(thumbnailScaled(QString,QString)), this, SLOT(thumbnailScaled(QString,QString)));
        mPostprocessWorkerThread->start();
    }
    if(pbs.getBool("printer", "enable")) {
        if(!mPrinterThread) {
            mPrinterThread = new QThread();
            mPrinterThreadObject = new printerWorker();
            mPrinterThreadObject->moveToThread(mPrinterThread);
            connect(mPrinterThreadObject, SIGNAL(finished()), mPrinterThread, SLOT(quit()));
            connect(mPrinterThreadObject, SIGNAL(finished()), mPrinterThread, SLOT(deleteLater()));
            connect(mPrinterThreadObject, SIGNAL(printerError(QString)), this, SLOT(printerError(QString)));
            connect(mPrinterThread, SIGNAL(started()), mPrinterThreadObject, SLOT(start()));
            connect(mPrinterThread, SIGNAL(finished()), mPrinterThreadObject, SLOT(deleteLater()));
            connect(this, SIGNAL(stopPrinterThread()), mPrinterThreadObject, SLOT(stop()));
            connect(this, SIGNAL(printPicture(QPixmap,int)), mPrinterThreadObject, SLOT(addPrintJob(QPixmap,int)));
            connect(this, SIGNAL(printPictureFile(QString,int)), mPrinterThreadObject, SLOT(addFilePrintJob(QString,int)));
            connect(this, SIGNAL(initPrinter()), mPrinterThreadObject, SLOT(initPrinter()));
            mPrinterThread->start();
        }
        emit initPrinter();
    }
    if(pbs.getBool("show", "enable")) {
        if(!mShowThread) {
            mShowThread = new QThread();
            mShowThreadObject = new showWorker();
            mShowThreadObject->moveToThread(mShowThread);
            connect(mShowThreadObject, SIGNAL(finished()), mShowThread, SLOT(quit()));
            connect(mShowThreadObject, SIGNAL(finished()), mShowThread, SLOT(deleteLater()));
            connect(mShowThread, SIGNAL(started()), mShowThreadObject, SLOT(start()));
            connect(mShowThread, SIGNAL(finished()), mShowThreadObject, SLOT(deleteLater()));
            connect(this, SIGNAL(stopShowThread()), mShowThreadObject, SLOT(stop()));
            connect(this, SIGNAL(addPictureToShow(QString)), mShowThreadObject, SLOT(addPicture(QString)));
            mShowThread->start();
        }
    }
    if(pbs.getBool("gpio", "enable")) {
        if(!mGpioThread) {
            StateMachine &sm = StateMachine::getInstance();
            mGpioThread = new QThread();
            mGpioThreadObject = new gpioWorker();
            mGpioThreadObject->moveToThread(mGpioThread);
            connect(mGpioThreadObject, SIGNAL(finished()), mGpioThread, SLOT(quit()));
            connect(mGpioThreadObject, SIGNAL(finished()), mGpioThread, SLOT(deleteLater()));
            connect(mGpioThreadObject, SIGNAL(gpioError(QString)), this, SLOT(genericError(QString)));
            connect(mGpioThread, SIGNAL(started()), mGpioThreadObject, SLOT(start()));
            connect(mGpioThread, SIGNAL(finished()), mGpioThreadObject, SLOT(deleteLater()));
            connect(this, SIGNAL(stopGpioThread()), mGpioThreadObject, SLOT(stop()));
            connect(&sm, SIGNAL(performStateChange(QString)), mGpioThreadObject, SLOT(setState(QString)));
            mGpioThread->start();
        }
    }
}

void MainWindow::loadSettingsToGui(bool showWindow)
{
    pbSettings &pbs = pbSettings::getInstance();
    mImagesToCapture = pbs.getInt("picture", "num_pictures");

    bool showEnabled = pbs.getBool("show", "enable");

    if(showEnabled) {
        QList<QScreen*> screens = qApp->screens();
        if(screens.size() == 1) {
            qDebug() << "Only one screen available, disabling show";
            pbs.setBool("show", "enable", false);
        } else {
            if(pbs.getBool("show", "swap_screens")) {
                setGeometry(screens.last()->geometry());
            } else {
                setGeometry(screens.first()->geometry());
            }
        }
    }

    if(pbs.getBool("screensaver", "enable")) {
        if(!mScreenSaver) {
            mScreenSaver = new screenSaver();
        }
        mScreenSaver->setTimeout(pbs.getInt("screensaver", "timeout"));
        mScreenSaver->setTargetState("idle");
        mScreenSaver->enableScreenSaver();
        StateMachine &stm = StateMachine::getInstance();
        connect(&stm, SIGNAL(performStateChange(QString)), mScreenSaver, SLOT(changeState(QString)));
    } else {
        if(mScreenSaver) {
            mScreenSaver->disableScreenSaver();
        }
    }

    bool fs = pbs.getBool("gui", "fullscreen");
    if(fs) {
        if(isVisible() || showWindow) {
            showFullScreen();
        }
    } else {
        int width = pbs.getInt("gui", "width");
        int height = pbs.getInt("gui", "height");
        setFixedSize(width, height);
        if(isVisible() || showWindow) {
            show();
        }
    }
    if(pbs.getBool("gui", "hide_cursor")) {
        qApp->setOverrideCursor(Qt::BlankCursor);
    } else {
        qApp->setOverrideCursor(Qt::ArrowCursor);
    }

    QString language = pbs.get("gui", "language");
    language.append(".qm");
    if(mTranslator.load(language)) {
        qApp->removeTranslator(&mTranslator);
        qApp->installTranslator(&mTranslator);
    } else {
        qDebug() << "error loading translation.";
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        qApp->quit();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if(mOverlayWidget) {
        mOverlayWidget->resize(event->size().width(), event->size().height());
    }
}

void MainWindow::errorOk()
{
    delete mOverlayWidget;
    mOverlayWidget = nullptr;
    mErrorPresent = false;
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState(sm.getCurrentState());
}

void MainWindow::errorQuit()
{
    delete mOverlayWidget;
    mOverlayWidget = nullptr;
    qApp->quit();
}

void MainWindow::errorRetry()
{
    delete mOverlayWidget;
    mOverlayWidget = nullptr;
    emit retryOperation();
    disconnect(this, SIGNAL(retryOperation()));
    mErrorPresent = false;
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState(sm.getCurrentState());
}

void MainWindow::imageCaptured(QPixmap image)
{
    pbSettings &pbs = pbSettings::getInstance();
    if(mImagesCaptured == 0) {
        emit initAssembleTask();
        mImageToPrint = "";
        mImageToReview = QPixmap();
    }
    emit addPicture(image);
    if(pbs.getBool("storage", "keep_pictures")) {
        emit saveFullPicture(image);
    }
    qDebug() << "Image captured.";
    mImagesCaptured++;
}

void MainWindow::pictureAssembled(QPixmap image)
{
    qDebug() << "Picture assembled.";
    mImageToReview = image;
    // FIXME: This should be triggered if the users chooses not to delete the image
    emit saveAssembledPicture(image);
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("review");
}

void MainWindow::printerError(QString error)
{
    mErrorPresent = true;
    delete mOverlayWidget;
    mOverlayWidget = new errorWidget(errorWidget::BTN_OK, error, this);
    mOverlayWidget->show();
    connect(mOverlayWidget, SIGNAL(errorOk()), this, SLOT(errorOk()));
}

void MainWindow::cameraError(QString error)
{
    mErrorPresent = true;
    delete mOverlayWidget;
    mOverlayWidget = new errorWidget(errorWidget::BTN_RETRY_QUIT, error, this);
    mOverlayWidget->show();
    connect(mOverlayWidget, SIGNAL(errorRetry()), this, SLOT(errorRetry()));
    connect(mOverlayWidget, SIGNAL(errorQuit()), this, SLOT(errorQuit()));
    connect(this, SIGNAL(retryOperation()), mCameraThreadObject, SLOT(retryOperation()));
}

void MainWindow::genericError(QString error)
{
    mErrorPresent = true;
    delete mOverlayWidget;
    mOverlayWidget = new errorWidget(errorWidget::BTN_OK_QUIT, error, this);
    mOverlayWidget->show();
    connect(mOverlayWidget, SIGNAL(errorOk()), this, SLOT(errorOk()));
    connect(mOverlayWidget, SIGNAL(errorQuit()), this, SLOT(errorQuit()));
}

void MainWindow::startPrintJob(int numcopies)
{
    qDebug() << "startPrintJob: " << numcopies;
    if(!mImageToPrint.isEmpty()) {
        emit printPictureFile(mImageToPrint, numcopies);
    } else {
        emit printPicture(mImageToReview, numcopies);
    }
}

void MainWindow::thumbnailScaled(QString path, QString filename)
{
    qDebug() << "Thumbnail scaled: " << path << QDir::separator() << filename;
    emit addPictureToShow(filename);
}

void MainWindow::fullImageSaved(QString path, QString filename, bool ret)
{

}

void MainWindow::assembledImageSaved(QString path, QString filename, bool ret)
{
    mImageToPrint = path + QDir::separator() + filename;
    if(ret)
        emit saveThumbnail(filename);
}

void MainWindow::removableDeviceDetected(QString path)
{
    qDebug() << "removableDeviceDetected";
    bool reload = false;
    pbSettings &settings = pbSettings::getInstance();

    QFile cfgFile(path + QDir::separator() + "qtphotobox.ini");
    if(cfgFile.exists()) {
        reload = true;
        settings.mergeConfigFile(cfgFile.fileName());
    }
    QFile bgFile(path + QDir::separator() + "background.jpg");
    if(bgFile.exists()) {
        reload = true;
        settings.set("picture", "background", bgFile.fileName());
    }
    if(reload) {
        loadSettingsToGui(true);
    }
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    QFontDatabase db;
    StateMachine &sm = StateMachine::getInstance();
    QObject::connect(&sm, SIGNAL(performStateChange(QString)),
            &w, SLOT(changeState(QString)));

    QString basepath;
#ifdef Q_WS_WIN
    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    if(environment.contains("APPDATA"))
    {
        configPath = environment.value("APPDATA");
    }
    else
    {
        basepath = QDir::homePath();
    }
#else
    basepath = QDir::homePath();
#endif
    if(!basepath.endsWith(QDir::separator(), Qt::CaseSensitive))
      basepath += '/';
#ifdef Q_WS_WIN
    basepath += "qtphotobox/";
#else
    basepath += ".qtphotobox/";
#endif
    if(!QDir(basepath).exists())
        QDir().mkpath(basepath);

    pbSettings &pbs = pbSettings::getInstance();
    pbs.setConfigPath(basepath);

    basepath += "settings.ini";
    QFile settingsFile(basepath);
    if(!settingsFile.exists()) {
        pbs.initDefaultConfg();
        pbs.saveConfigFile(basepath);
    }
    pbs.mergeConfigFile(basepath);

    QString styleName = pbs.get("gui", "style");
    qDebug() << styleName;

    if(styleName != "default") {
        QFile styleFile(":/" + styleName + ".qss");
        if(styleFile.exists()) {
            styleFile.open(QIODevice::ReadOnly);
            app.setStyleSheet(QString(styleFile.readAll()));
            styleFile.close();
        }
    }

    // Add fonts
    QStringList fontlist;
    fontlist.append(":/gui/styles/fonts/AmaticSC-Bold.ttf");
    fontlist.append(":/gui/styles/fonts/AmaticSC-Regular.ttf");
    foreach(QString font, fontlist) {
        QFile fontFile(font);
        if(fontFile.exists()) {
            fontFile.open(QIODevice::ReadOnly);
            db.addApplicationFontFromData(fontFile.readAll());
            fontFile.close();
        }
    }

    QString startupState = "start";
    if(pbs.getBool("gui", "direct_start")) {
        startupState = "init";
    }

    sm.addState("start");
    sm.addState("waitremovable");
    sm.addState("init");
    sm.addState("settings");
    sm.addState("idle");
    sm.addState("archive");
    sm.addState("greeter");
    sm.addState("countdown");
    sm.addState("capture");
    sm.addState("assemble");
    sm.addState("review");
    sm.addState("postprocess");
    sm.addState("error");
    sm.addState("teardown");
    sm.addState("restart");
    sm.addState("screensaver");

    if(pbs.getBool("storage", "wait_removable")) {
        sm.addTargetState("start", "waitremovable");
        sm.addTargetState("waitremovable", "init");
        if(pbs.getBool("gui", "direct_start")) {
            startupState = "waitremovable";
        }
    } else {
        sm.addTargetState("start", "init");
    }
    sm.addTargetState("init", "idle");
    sm.addTargetState("archive", "idle");
    sm.addTargetState("idle", "greeter");
    sm.addTargetState("greeter", "countdown");
    sm.addTargetState("countdown", "capture");
    sm.addTargetState("capture", "assemble");
    sm.addTargetState("assemble", "review");
    sm.addTargetState("review", "postprocess");
    sm.addTargetState("postprocess", "idle");
    sm.addTargetState("screensaver", "idle");

    sm.triggerState(startupState);

    w.loadSettingsToGui(true);
    //QMetaObject::invokeMethod(&w, "loadSettingsToGui", Qt::QueuedConnection, Q_ARG(bool, true));
    return app.exec();
}

