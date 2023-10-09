#include "pbcamera.h"
#include "camera_dummy.h"
#include "settings.h"
#include "camera_interface.h"
#include <QTimer>
#include <QEventLoop>
#include <QDebug>
#include <QThread>

#ifdef BUILD_GPHOTO2
#include "camera_gphoto2.h"
#endif

#ifdef BUILD_GENERIC_CAMERA
#include "camera_generic.h"
#endif

pbCamera::pbCamera() {
    mCaptureCamera = nullptr;
    mPreviewCamera = nullptr;
    mLimitFps = false;
    mLimitTimer = nullptr;
    mInitialized = false;
}

pbCamera::~pbCamera() {
    if(mCaptureCamera != mPreviewCamera) {
        delete mPreviewCamera;
    }
    mPreviewCamera = nullptr;
    delete mCaptureCamera;
    mCaptureCamera = nullptr;
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
    QString captureBackend = pbs.get("camera", "capturebackend");
    QString previewBackend = pbs.get("camera", "previewbackend");
    QString previewName = pbs.get("camera", "previewname");
    QString captureName = pbs.get("camera", "capturename");

    bool flipPreview = pbs.getBool("camera", "previewflip");
    bool flipCapture = pbs.getBool("camera", "captureflip");

    int fps = pbs.getInt("camera", "fps");
    if(fps > 0) {
        mLimitFps = true;
    } else {
        mLimitFps = false;
    }
    int previewRotation = pbs.getInt("camera", "previewrotation");
    int captureRotation = pbs.getInt("camera", "capturerotation");

    bool portraitPreview = pbs.getBool("camera", "previewportrait");
    bool portraitCapture = pbs.getBool("camera", "captureportrait");

    int previewCropW = 0;
    int previewCropH = 0;
    int previewCropO = 0;
    int captureCropW = 0;
    int captureCropH = 0;
    int captureCropO = 0;


#ifdef BUILD_GPHOTO2
    if(previewBackend == "gphoto2") {
        if(mPreviewCamera)
            delete mPreviewCamera;
        mPreviewCamera = new CameraGphoto2();
    }
#endif
#ifdef BUILD_GENERIC_CAMERA
    if(previewBackend == "generic") {
        if(mPreviewCamera)
            delete mPreviewCamera;
        mPreviewCamera = new CameraGeneric();
        mPreviewCamera->setCameraName(previewName);
    }
#endif
    if(previewBackend == "dummy") {
        if(mPreviewCamera)
            delete mPreviewCamera;
        mPreviewCamera = new CameraDummy();
    }

#ifdef BUILD_GENERIC_CAMERA
    if(captureBackend == "generic" && previewBackend == "generic") {
        if(mCaptureCamera)
            delete mCaptureCamera;
        if(captureName != previewName) {
            mCaptureCamera = new CameraGeneric();
            mCaptureCamera->setCameraName(captureName);
        } else {
            mCaptureCamera = mPreviewCamera;
        }
    } else if(captureBackend == previewBackend) {
#else
    if(captureBackend == previewBackend) {
#endif
        if(mCaptureCamera)
            delete mCaptureCamera;
        mCaptureCamera = mPreviewCamera;
    } else {
#ifdef BUILD_GPHOTO2
        if(captureBackend == "gphoto2") {
            if(mCaptureCamera)
                delete mCaptureCamera;
            mCaptureCamera = new CameraGphoto2();
        }
#endif
#ifdef BUILD_GENERIC_CAMERA
        if(captureBackend == "generic") {
            if(mCaptureCamera)
                delete mCaptureCamera;
            mCaptureCamera = new CameraGeneric();
            mCaptureCamera->setCameraName(captureName);
        }
#endif
        if(captureBackend == "dummy") {
            if(mCaptureCamera)
               delete mCaptureCamera;
            mCaptureCamera = new CameraDummy();
        }
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

        mMutex.lock();
        QString command = mCommandList.takeFirst();
        mMutex.unlock();
        if(command == "initCamera" || command == "retryOperation") {
            if(mPreviewCamera && !mInitialized) {
                if(mPreviewCamera != mCaptureCamera) {
                    bool ret = mPreviewCamera->initCamera();
                    if(!ret) {
                        emit cameraError(tr("Error initializing preview camera. Check connection. "));
                        continue;
                    }
                }
            }
            if(mCaptureCamera && !mInitialized) {
                bool ret = mCaptureCamera->initCamera();
                if(!ret) {
                    emit cameraError(tr("Error initializing capture camera. Check connection."));
                } else {
                    mCaptureCamera->setIdle();
                    QPixmap testshot = mCaptureCamera->getCaptureImage();
                    if(captureRotation != 0) {
                        QTransform transform = QTransform().rotate(captureRotation);
                        testshot = testshot.transformed(transform);
                    }
                    if(portraitCapture) {
                        captureCropH = testshot.size().height();
                        float ratio = testshot.size().width() / (float)testshot.size().height();
                        captureCropW = captureCropH / ratio;
                        captureCropO = (testshot.size().width() - captureCropW) / 2;
                        testshot = testshot.copy(captureCropO, 0, captureCropW, captureCropH);
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
                    mPreviewCamera->setActive();
                    mLimitTimer->start();
                }
                QPixmap image = mPreviewCamera->getPreviewImage();
                if(image.isNull()) {
                    emit cameraError(tr("Error capturing image. Camera connected?"));
                } else {
                    if(flipPreview)
                        image = image.transformed(QTransform().scale(-1, 1));
                    if(previewRotation != 0) {
                        QTransform transform = QTransform().rotate(previewRotation);
                        image = image.transformed(transform);
                    }
                    if(portraitPreview) {
                        if(previewCropO == 0) {
                            previewCropH = image.size().height();
                            float ratio = image.size().width() / (float)image.size().height();
                            previewCropW = previewCropH / ratio;
                            previewCropO = (image.size().width() - previewCropW) / 2;
                        }
                        image = image.copy(previewCropO, 0, previewCropW, previewCropH);
                    }
                    emit previewImageCaptured(image);
                }
            } else {
                mPreviewCamera->setActive();
                // Check for stopPreview
                while(!checkForNewCommand()) {
                    QPixmap image = mPreviewCamera->getPreviewImage();
                    if(image.isNull()) {
                        emit cameraError(tr("Error capturing image. Camera connected?"));
                    } else {
                        if(flipPreview)
                            image = image.transformed(QTransform().scale(-1, 1));
                        if(previewRotation != 0) {
                            QTransform transform = QTransform().rotate(previewRotation);
                            image = image.transformed(transform);
                        }
                        if(portraitPreview) {
                            if(previewCropO == 0) {
                                previewCropH = image.size().height();
                                float ratio = image.size().width() / (float)image.size().height();
                                previewCropW = previewCropH / ratio;
                                previewCropO = (image.size().width() - previewCropW) / 2;
                            }
                            image = image.copy(previewCropO, 0, previewCropW, previewCropH);
                        }
                        emit previewImageCaptured(image);
                    }
                }
                mPreviewCamera->setIdle();
            }
        } else if(command == "stopPreview") {
            if(mLimitFps) {
                mPreviewCamera->setIdle();
            }
        } else if(command == "captureImage") {
            mCaptureCamera->setIdle();
            QPixmap image = mCaptureCamera->getCaptureImage();
            if(image.isNull()) {
                emit cameraError(tr("Error capturing image. Camera connected?"));
            } else {
                if(flipCapture)
                    image = image.transformed(QTransform().scale(-1, 1));
                if(captureRotation != 0) {
                    QTransform transform = QTransform().rotate(captureRotation);
                    image = image.transformed(transform);
                }
                if(portraitCapture) {
                    if(captureCropO == 0) {
                        captureCropH = image.size().height();
                        float ratio = image.size().width() / (float)image.size().height();
                        captureCropW = captureCropH / ratio;
                        captureCropO = (image.size().width() - captureCropW) / 2;
                    }
                    image = image.copy(captureCropO, 0, previewCropW, captureCropH);
                }
                emit imageCaptured(image);
            }
        } else if(command == "stopThread") {
            mCaptureCamera->setIdle();
            if(mPreviewCamera != mCaptureCamera)
                mPreviewCamera->setIdle();
            running = false;
        }
    }

    if(mPreviewCamera != mCaptureCamera) {
        delete mPreviewCamera;
    }
    mPreviewCamera = nullptr;
    delete mCaptureCamera;
    mCaptureCamera = nullptr;
    emit finished();
}

void pbCamera::stop()
{
    qDebug() << "Stop" << QThread::currentThreadId();
    mMutex.lock();
    mCommandList.append("stopThread");
    mMutex.unlock();
}

void pbCamera::startPreview()
{
    qDebug() << "startPreview";
    mMutex.lock();
    mCommandList.append("startPreview");
    mMutex.unlock();
}

void pbCamera::stopPreview()
{
    qDebug() << "stopPreview";
    if(mLimitFps) {
        mLimitTimer->stop();
    }
    mMutex.lock();
    mCommandList.append("stopPreview");
    mMutex.unlock();
}

void pbCamera::retryOperation()
{
    qDebug() << "retryOperation";
    mMutex.lock();
    mCommandList.append("retryOperation");
    mMutex.unlock();
}

void pbCamera::captureImage()
{
    qDebug() << "captureImage";
    mMutex.lock();
    mCommandList.append("captureImage");
    mMutex.unlock();
}
