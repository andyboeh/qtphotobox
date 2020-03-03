#include "camera_interface.h"

CameraInterface::CameraInterface()
{

}

CameraInterface::~CameraInterface()
{

}

bool CameraInterface::initCamera()
{
    return false;
}

QPixmap CameraInterface::getPreviewImage()
{
    return QPixmap();
}

QPixmap CameraInterface::getCaptureImage()
{
    return QPixmap();
}

void CameraInterface::setActive()
{

}

void CameraInterface::setIdle()
{

}
