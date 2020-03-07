#include "pbcamera.h"
#include "camera_gphoto2.h"
#include "camera_dummy.h"
#include "settings.h"
#include "camera_interface.h"
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <QThread>

pbCamera::pbCamera() {
    mCamera = nullptr;
    mLimitFps = true;
    mLimitTimer = nullptr;
}

pbCamera::~pbCamera() {
    delete mCamera;
    mCamera = nullptr;
    delete mLimitTimer;
    mLimitTimer = nullptr;
}

void pbCamera::initCamera()
{
    qDebug() << "initCamera";
    mCommandList.append("initCamera");
}

void pbCamera::start()
{
    qDebug() << QThread::currentThreadId();
    qDebug() << "Thread start.";

    pbSettings &pbs = pbSettings::getInstance();
    QString backend = pbs.get("camera", "backend");
    int fps = pbs.getInt("camera", "fps");
    if(fps > 0) {
        mLimitFps = true;
    }

    if(backend == "gphoto2") {
        if(mCamera)
            delete mCamera;
        mCamera = new CameraGphoto2();
    } else if(backend == "dummy") {
        if(mCamera)
            delete mCamera;
        mCamera = new CameraDummy();
    }

    QEventLoop loop;
    bool running = true;

    if(!mLimitTimer) {
        mLimitTimer = new QTimer();
        mLimitTimer->setInterval(1000 / fps);
        mLimitTimer->setSingleShot(false);
        connect(mLimitTimer, SIGNAL(timeout()), this, SLOT(startPreview()));
    }

    emit started();

    while(running) {
        waitForCommand();
        if(mCommandList.isEmpty())
            continue;
        QString command = mCommandList.takeFirst();
        if(command == "initCamera" || command == "retryOperation") {
            if(mCamera) {
                bool ret = mCamera->initCamera();
                if(ret)
                    mCamera->setIdle();
                else
                    emit cameraError(tr("Error initializing camera. Check connection."));
                emit cameraInitialized(ret);
            }
        } else if(command == "startPreview") {
            if(mLimitFps) {
                if(!mLimitTimer->isActive()) {
                    mCamera->setActive();
                    mLimitTimer->start();
                }
                QPixmap image = mCamera->getPreviewImage();
                if(image.isNull())
                    emit cameraError(tr("Error capturing image. Camera connected?"));
                else
                    emit previewImageCaptured(image);
            } else {
                mCamera->setActive();
                // Check for stopPreview
                while(!checkForNewCommand()) {
                    QPixmap image = mCamera->getPreviewImage();
                    if(image.isNull())
                        emit cameraError(tr("Error capturing image. Camera connected?"));
                    else
                        emit previewImageCaptured(image);
                }
                mCamera->setIdle();
            }
        } else if(command == "stopPreview") {
            if(mLimitFps && mLimitTimer->isActive()) {
                mLimitTimer->stop();
                mCamera->setIdle();
            }
        } else if(command == "captureImage") {
            mCamera->setActive();
            QPixmap image = mCamera->getCaptureImage();
            if(image.isNull())
                emit cameraError(tr("Error capturing image. Camera connected?"));
            else
                emit imageCaptured(image);
            mCamera->setIdle();
        } else if(command == "stopThread") {
            running = false;
        }
    }

    delete mCamera;
    mCamera = nullptr;
    emit finished();
}

void pbCamera::stop()
{
    qDebug() << "Stop" << QThread::currentThreadId();
    mCommandList.append("stopThread");
}

void pbCamera::startPreview()
{
    qDebug() << "startPreview";
    mCommandList.append("startPreview");
}

void pbCamera::stopPreview()
{
    qDebug() << "stopPreview";
    mCommandList.append("stopPreview");
}

void pbCamera::retryOperation()
{
    qDebug() << "retryOperation";
    mCommandList.append("retryOperation");
}

void pbCamera::captureImage()
{
    qDebug() << "captureImage";
    mCommandList.append("captureImage");
}
