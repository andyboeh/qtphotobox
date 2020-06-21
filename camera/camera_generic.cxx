#include "camera_generic.h"
#include "camera_generic_surface.h"
#include "settings.h"
#include <QCameraInfo>
#include <QDebug>
#include <QCameraImageCapture>
#include <QEventLoop>
#include <QTimer>

CameraGeneric::CameraGeneric()
{
    mCamera = nullptr;
    mSurface = nullptr;
    mCapture = nullptr;
}

CameraGeneric::~CameraGeneric()
{
    if(mCamera) {
        delete mCamera;
        mCamera = nullptr;
    }
    if(mSurface) {
        delete mSurface;
        mSurface = nullptr;
    }
    if(mCapture) {
        delete mCapture;
        mCapture = nullptr;
    }
}

bool CameraGeneric::initCamera()
{
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    int camid = 0;

    if(cameras.length() == 0) {
        return false;
    }

    if(!mCameraName.isEmpty()) {
        for(int i=0; i<cameras.length(); i++) {
            if(cameras.at(i).description().contains(mCameraName)) {
                qDebug() << "Found camera: " << cameras.at(i).description();
                camid = i;
                break;
            }
        }
    }
    if(mCamera) {
        delete mCamera;
        mCamera = nullptr;
    }
    if(mSurface) {
        delete mSurface;
        mSurface = nullptr;
    }
    if(mCapture) {
        delete mCapture;
        mCapture = nullptr;
    }
    mCamera = new QCamera(cameras.at(camid));
    mSurface = new CameraGenericSurface();
    mCapture = new QCameraImageCapture(mCamera);
    connect(mCapture, SIGNAL(imageCaptured(int,QImage)), this, SLOT(imageCaptured(int,QImage)));
    connect(mCapture, SIGNAL(readyForCaptureChanged(bool)), this, SLOT(readyForCaptureChanged(bool)));
    mCapture->setCaptureDestination(QCameraImageCapture::CaptureToBuffer);

    mCamera->setViewfinder(mSurface);
    return true;
}

QPixmap CameraGeneric::getPreviewImage()
{
    return mSurface->getCurrentFrame();
}

QPixmap CameraGeneric::getCaptureImage()
{
    qDebug() << "getCaptureImage";
    mCamera->setCaptureMode(QCamera::CaptureStillImage);
    mCamera->start();
    waitForActiveState();
    qDebug() << "done";
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(10000);
    timeout.start();
    connect(this, SIGNAL(newImageCaptured()), &loop, SLOT(quit()));
    connect(&timeout, SIGNAL(timeout()), &loop, SLOT(quit()));
    loop.exec();
    mCamera->stop();
    return mLastImage;
}

void CameraGeneric::readyForCaptureChanged(bool state) {
    qDebug() << "readyForCaptureChanged";
    if(state) {
        qDebug() << "true";
        mCamera->searchAndLock();
        mCapture->capture();
        mCamera->unlock();
    }
}

void CameraGeneric::imageCaptured(int id, QImage img) {
    mLastImage = QPixmap::fromImage(img);
    emit newImageCaptured();
}

bool CameraGeneric::waitForActiveState() {
    qDebug() << "waitForActiveState";
    QCamera::Status status = mCamera->status();
    if(status == QCamera::ActiveStatus)
        return true;

    QEventLoop loop;
    connect(mCamera, SIGNAL(statusChanged(QCamera::Status)), &loop, SLOT(quit()));
    loop.exec();
    status = mCamera->status();
    if(status == QCamera::ActiveStatus)
        return true;
    if(status == QCamera::StartingStatus)
        return waitForActiveState();
    return false;
}

void CameraGeneric::setIdle()
{
    mCamera->stop();
}

void CameraGeneric::setActive()
{
    mCamera->setCaptureMode(QCamera::CaptureViewfinder);
    mCamera->start();
    waitForActiveState();
}

QStringList CameraGeneric::getCameraNames()
{
    QStringList ret;
    QList<QCameraInfo> cameras = QCameraInfo::availableCameras();
    for(int i=0; i<cameras.length(); i++) {
        ret.append(cameras.at(i).description());
    }
    return ret;
}
