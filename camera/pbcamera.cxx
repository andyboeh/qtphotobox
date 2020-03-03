#include "pbcamera.h"
#include "camera_gphoto2.h"
#include "settings.h"
#include "camera_interface.h"
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <QThread>

pbCamera::pbCamera() {
    mCamera = nullptr;
    mCommand = "";
}

pbCamera::~pbCamera() {
    delete mCamera;
    mCamera = nullptr;
}

bool pbCamera::initCamera()
{
    qDebug() << QThread::currentThreadId();
    bool ret = false;

    if(mCamera) {
        ret = mCamera->initCamera();
        if(ret)
            mCamera->setIdle();
    }
    emit cameraInitialized(ret);
    return ret;
}

bool pbCamera::checkForNewCommand() {
    auto const dispatcher = QThread::currentThread()->eventDispatcher();
    if(!dispatcher) {
        qCritical() << "thread with no dispatcher.";
        return false;
    }
    dispatcher->processEvents(QEventLoop::AllEvents);
    return !mCommand.isEmpty();
}

void pbCamera::start()
{
    if(state == PAUSED)
        state = RUNNING;

    if(state == RUNNING)
        return;

    state = RUNNING;

    qDebug() << QThread::currentThreadId();
    qDebug() << "Thread start.";

    pbSettings &pbs = pbSettings::getInstance();
    QString backend = pbs.get("camera", "backend");

    if(backend == "gphoto2") {
        if(mCamera)
            delete mCamera;
        mCamera = new CameraGphoto2();
    }

    while(state == RUNNING) {
        auto const dispatcher = QThread::currentThread()->eventDispatcher();
        if(!dispatcher) {
            qCritical() << "thread with no dispatcher.";
            return;
        }
        while(mCommand.isEmpty()) {
            dispatcher->processEvents(QEventLoop::WaitForMoreEvents);
        }
        if(mCommand == "startPreview") {
            mCommand = "";
            mCamera->setActive();
            // Check for stopPreview
            while(!checkForNewCommand()) {
                QPixmap image = mCamera->getPreviewImage();
                emit previewImageCaptured(image);
            }
            mCamera->setIdle();
        } else if(mCommand == "stopPreview") {
            mCommand = "";
        } else if(mCommand == "captureImage") {
            mCommand = "";
            mCamera->setActive();
            QPixmap image = mCamera->getCaptureImage();
            emit imageCaptured(image);
            mCamera->setIdle();
        } else if(mCommand == "stop") {
            state = IDLE;
        }
    }

    delete mCamera;
    mCamera = nullptr;
    emit finished();
}

void pbCamera::stop()
{
    qDebug() << "Stop" << QThread::currentThreadId();
    mCommand = "stop";
}

void pbCamera::startPreview()
{
    qDebug() << "startPreview";
    mCommand = "startPreview";
}

void pbCamera::stopPreview()
{
    qDebug() << "stopPreview";
    mCommand = "stopPreview";
}

void pbCamera::captureImage()
{
    qDebug() << "captureImage";
    mCommand = "captureImage";
}
