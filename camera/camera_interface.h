#ifndef _CAMERA_INTERFACE_H
#define _CAMERA_INTERFACE_H

#include <QPixmap>

class CameraInterface {
public:
    CameraInterface();
    virtual ~CameraInterface();
    virtual bool initCamera();
    virtual QPixmap getPreviewImage();
    virtual QPixmap getCaptureImage();
    virtual void setActive();
    virtual void setIdle();
};

#endif //_CAMERA_INTERFACE_H