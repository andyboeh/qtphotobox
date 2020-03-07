#include "photobooth-rewrite.h"
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
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QFontDatabase>
#include <QWindow>
#include <QScreen>
#include <QKeyEvent>

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
    mImagesCaptured = 0;
    mImagesToCapture = 0;
    mFullscreen = false;
}

MainWindow::~MainWindow()
{
    if(mCameraThread) {
        emit stopCameraThread();
        mCameraThread->quit();
        mCameraThread->wait();
    }
    if(mPictureWorkerThread) {
        emit stopPictureWorkerThread();
        mPictureWorkerThread->quit();
        mPictureWorkerThread->wait();
    }
    if(mPostprocessWorkerThread) {
        emit stopPostprocessWorkerThread();
        mPostprocessWorkerThread->quit();
        mPostprocessWorkerThread->wait();
    }
    if(mPrinterThread) {
        emit stopPrinterThread();
        mPrinterThread->quit();
        mPrinterThread->wait();
    }
    if(mShowThread) {
        emit stopShowThread();
        mShowThread->quit();
        mShowThread->wait();
    }
    delete mCurrentWidget;
    mCurrentWidget = nullptr;
}

void MainWindow::changeState(QString name)
{
    StateMachine &sm = StateMachine::getInstance();
    qDebug() << name;
    if(mOverlayWidget) {
        delete mOverlayWidget;
        mOverlayWidget = nullptr;
    }
    if(name == "start") {
        delete mCurrentWidget;
        mCurrentWidget = new startWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "init") {
        qDebug() << QThread::currentThreadId();
        if(!mCameraThread) {
            mCameraThread = new QThread();
            mCameraThreadObject = new pbCamera();
            mCameraThreadObject->moveToThread(mCameraThread);
            connect(mCameraThreadObject, SIGNAL(finished()), mCameraThread, SLOT(quit()));
            connect(mCameraThreadObject, SIGNAL(finished()), mCameraThread, SLOT(deleteLater()));
            connect(mCameraThreadObject, SIGNAL(imageCaptured(QPixmap)), this, SLOT(imageCaptured(QPixmap)));
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
            connect(mPostprocessWorkerThreadObject, SIGNAL(fullImageSaved(QString,bool)), this, SLOT(fullImageSaved(QString,bool)));
            connect(mPostprocessWorkerThreadObject, SIGNAL(assembledImageSaved(QString,bool)), this, SLOT(assembledImageSaved(QString,bool)));
            connect(mPostprocessWorkerThread, SIGNAL(started()), mPostprocessWorkerThreadObject, SLOT(start()));
            connect(mPostprocessWorkerThread, SIGNAL(finished()), mPostprocessWorkerThreadObject, SLOT(deleteLater()));
            connect(this, SIGNAL(stopPostprocessWorkerThread()), mPostprocessWorkerThreadObject, SLOT(stop()));
            connect(this, SIGNAL(saveFullPicture(QPixmap)), mPostprocessWorkerThreadObject, SLOT(saveFullImage(QPixmap)));
            connect(this, SIGNAL(saveAssembledPicture(QPixmap)), mPostprocessWorkerThreadObject, SLOT(saveAssembledImage(QPixmap)));
            connect(this, SIGNAL(saveThumbnail(QString)), mPostprocessWorkerThreadObject, SLOT(saveThumbnail(QString)));
            connect(mPostprocessWorkerThreadObject, SIGNAL(thumbnailScaled(QString)), this, SLOT(thumbnailScaled(QString)));
            mPostprocessWorkerThread->start();
        }
        pbSettings &pbs = pbSettings::getInstance();
        if(pbs.getBool("printer", "enable")) {
            if(!mPrinterThread) {
                mPrinterThread = new QThread();
                mPrinterThreadObject = new printerThreadObject();
                mPrinterThreadObject->moveToThread(mPrinterThread);
                connect(mPrinterThreadObject, SIGNAL(finished()), mPrinterThread, SLOT(quit()));
                connect(mPrinterThreadObject, SIGNAL(finished()), mPrinterThread, SLOT(deleteLater()));
                connect(mPrinterThread, SIGNAL(started()), mPrinterThreadObject, SLOT(start()));
                connect(mPrinterThread, SIGNAL(finished()), mPrinterThreadObject, SLOT(deleteLater()));
                connect(this, SIGNAL(stopPrinterThread()), mPrinterThreadObject, SLOT(stop()));
                connect(this, SIGNAL(printPicture(QPixmap,int)), mPrinterThreadObject, SLOT(addPrintJob(QPixmap,int)));
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
                mShowThread->start();
            }
        }
        delete mCurrentWidget;
        mCurrentWidget = new initWidget(mCameraThreadObject);
        setCentralWidget(mCurrentWidget);
    } else if(name == "settings") {
        delete mCurrentWidget;
        mCurrentWidget = new settingsWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "idle") {
        delete mCurrentWidget;
        mCurrentWidget = new idleWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "archive") {
        delete mCurrentWidget;
        mCurrentWidget = new archiveWidget();
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

    bool fs = pbs.getBool("gui", "fullscreen");
    if(fs) {
        mFullscreen = true;
        if(isVisible() || showWindow) {
            hide();
            showFullScreen();
        }
    } else {
        mFullscreen = false;
        int width = pbs.getInt("gui", "width");
        int height = pbs.getInt("gui", "height");
        setFixedSize(width, height);
        if(isVisible() || showWindow) {
            hide();
            show();
        }
    }
    if(pbs.getBool("gui", "hide_cursor")) {
        qApp->setOverrideCursor(Qt::BlankCursor);
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Escape) {
        qApp->quit();
    }
}

void MainWindow::imageCaptured(QPixmap image)
{
    pbSettings &pbs = pbSettings::getInstance();
    if(mImagesCaptured == 0) {
        emit initAssembleTask();
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

}

void MainWindow::startPrintJob(int numcopies)
{
    qDebug() << "startPrintJob: " << numcopies;
    emit printPicture(mImageToReview, numcopies);
}

void MainWindow::thumbnailScaled(QString path)
{
    qDebug() << "Thumbnail scaled: " << path;
}

void MainWindow::fullImageSaved(QString filename, bool ret)
{

}

void MainWindow::assembledImageSaved(QString filename, bool ret)
{
    if(ret)
        emit saveThumbnail(filename);
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
    basepath += "photobooth-rewrite/";
#else
    basepath += ".photobooth-rewrite/";
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

    sm.addState("start");
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

    sm.addTargetState("start", "init");
    sm.addTargetState("init", "idle");
    sm.addTargetState("archive", "idle");
    sm.addTargetState("idle", "greeter");
    sm.addTargetState("greeter", "countdown");
    sm.addTargetState("countdown", "capture");
    sm.addTargetState("capture", "assemble");
    sm.addTargetState("assemble", "review");
    sm.addTargetState("review", "postprocess");
    sm.addTargetState("postprocess", "idle");

    sm.triggerState("start");

    // Temporary
    storageManager &stm = storageManager::getInstance();
    stm.waitForRemovableDevice();

    w.loadSettingsToGui(true);
    //QMetaObject::invokeMethod(&w, "loadSettingsToGui", Qt::QueuedConnection, Q_ARG(bool, true));
    return app.exec();
}

