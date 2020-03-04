#include "camera_dummy.h"

CameraDummy::CameraDummy()
{

}

CameraDummy::~CameraDummy()
{

}

bool CameraDummy::initCamera()
{
    return true;
}

QPixmap CameraDummy::getPreviewImage()
{
    return getCaptureImage();
}

QPixmap CameraDummy::getCaptureImage()
{
    QPixmap image(1920,1280);
    mHue = (mHue + 1) % 360;
    image.fill(QColor::fromHsvF(mHue / 360.0, 0.2, 0.9));
    return image;
}

void CameraDummy::setIdle()
{

}

void CameraDummy::setActive()
{

}
