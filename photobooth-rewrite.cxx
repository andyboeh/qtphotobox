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
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QThread>
#include <QFontDatabase>

MainWindow::MainWindow()
{
    mCurrentWidget = nullptr;
    mCameraThread = nullptr;
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
    delete mCurrentWidget;
    mCurrentWidget = nullptr;
}

void MainWindow::changeState(QString name)
{
    StateMachine &sm = StateMachine::getInstance();
    qDebug() << name;
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
            connect(mCameraThreadObject, SIGNAL(imageCaptured(QPixmap)), this, SLOT(imageCaptured()));
            connect(mCameraThread, SIGNAL(started()), mCameraThreadObject, SLOT(start()));
            connect(mCameraThread, SIGNAL(finished()), mCameraThreadObject, SLOT(deleteLater()));
            connect(this, SIGNAL(stopCameraThread()), mCameraThreadObject, SLOT(stop()));
            mCameraThread->start();
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
            delete mCurrentWidget;
            mCurrentWidget = new assembleWidget();
            setCentralWidget(mCurrentWidget);
            qDebug() << "All images captured, assembling...";
        }
    } else if(name == "review") {
        delete mCurrentWidget;
        mCurrentWidget = new reviewWidget();
        setCentralWidget(mCurrentWidget);
    } else if(name == "error") {

    } else if(name == "teardown") {
        QApplication::quit();
    }
}

void MainWindow::loadSettingsToGui(bool showWindow)
{
    pbSettings &pbs = pbSettings::getInstance();
    int numx = pbs.getInt("picture", "num_x");
    int numy = pbs.getInt("picture", "num_y");
    int numskip;
    QString skip = pbs.get("picture", "skip");
    if(skip.isEmpty()) {
        numskip = 0;
    } else {
        numskip = skip.split(",").length();
    }

    mImagesToCapture = numx * numy - numskip;

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
}

void MainWindow::imageCaptured()
{
    qDebug() << "Image captured.";
    mImagesCaptured++;
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

    storageManager &stm = storageManager::getInstance();
    stm.waitForRemovableDevice(false);

    w.loadSettingsToGui(true);
    //QMetaObject::invokeMethod(&w, "loadSettingsToGui");
    return app.exec();
}

