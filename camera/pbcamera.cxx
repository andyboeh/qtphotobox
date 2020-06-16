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
    mLimitFps = false;
    mLimitTimer = nullptr;
    mInitialized = false;
}

pbCamera::~pbCamera() {
    delete mCamera;
    mCamera = nullptr;
    if(mLimitTimer) {
        mLimitTimer->deleteLater();
    }
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
    } else {
        mLimitFps = false;
    }
    int rotation = pbs.getInt("camera", "rotation");

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

    if(!mLimitTimer && mLimitFps) {
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
            if(mCamera && !mInitialized) {
                bool ret = mCamera->initCamera();
                if(!ret) {
                    emit cameraError(tr("Error initializing camera. Check connection."));
                } else {
                    mCamera->setIdle();
                    QPixmap testshot = mCamera->getCaptureImage();
                    if(rotation != 0) {
                        QTransform transform = QTransform().rotate(rotation);
                        testshot = testshot.transformed(transform);
                    }
                    if(testshot.isNull()) {
                        emit cameraError(tr("Error capturing image. Camera connected?"));
                    } else {
                        emit testshotCaptured(testshot);
                    }
                    mInitialized = true;
                }
                emit cameraInitialized(ret);
            }
        } else if(command == "startPreview") {
            if(mLimitFps) {
                if(!mLimitTimer->isActive()) {
                    mCamera->setActive();
                    mLimitTimer->start();
                }
                QPixmap image = mCamera->getPreviewImage().transformed(QTransform().scale(-1, 1));
                if(image.isNull()) {
                    emit cameraError(tr("Error capturing image. Camera connected?"));
                } else {
                    if(rotation != 0) {
                        QTransform transform = QTransform().rotate(rotation);
                        image = image.transformed(transform);
                    }
                    emit previewImageCaptured(image);
                }
            } else {
                mCamera->setActive();
                // Check for stopPreview
                while(!checkForNewCommand()) {
                    QPixmap image = mCamera->getPreviewImage().transformed(QTransform().scale(-1, 1));
                    if(image.isNull()) {
                        emit cameraError(tr("Error capturing image. Camera connected?"));
                    } else {
                        if(rotation != 0) {
                            QTransform transform = QTransform().rotate(rotation);
                            image = image.transformed(transform);
                        }
                        emit previewImageCaptured(image);
                    }
                }
                mCamera->setIdle();
            }
        } else if(command == "stopPreview") {
            if(mLimitFps && mLimitTimer->isActive()) {
                mLimitTimer->stop();
                mCamera->setIdle();
            }
        } else if(command == "captureImage") {
            mCamera->setIdle();
            QPixmap image = mCamera->getCaptureImage();
            if(image.isNull()) {
                emit cameraError(tr("Error capturing image. Camera connected?"));
            } else {
                if(rotation != 0) {
                    QTransform transform = QTransform().rotate(rotation);
                    image = image.transformed(transform);
                }
                emit imageCaptured(image);
            }
        } else if(command == "stopThread") {
            mCamera->setIdle();
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
