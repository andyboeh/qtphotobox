#ifndef _CAMERA_DUMMY_H
#define _CAMERA_DUMMY_H

#include "camera_interface.h"
#include <QPixmap>
#include <QString>

class CameraDummy : public CameraInterface {
public:
    CameraDummy();
    ~CameraDummy();
    bool initCamera();
    QPixmap getPreviewImage();
    QPixmap getCaptureImage();
    void setIdle();
    void setActive();
private:
    int mHue;
};

#endif //_CAMERA_DUMMY_H
