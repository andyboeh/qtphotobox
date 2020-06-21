#ifndef _CAMERA_GENERIC_H
#define _CAMERA_GENERIC_H

#include "camera_interface.h"
#include <QPixmap>
#include <QObject>

class QCamera;
class CameraGenericSurface;
class QCameraImageCapture;

class CameraGeneric : public CameraInterface {
    Q_OBJECT
public:
    CameraGeneric();
    ~CameraGeneric();
    bool initCamera();
    QPixmap getPreviewImage();
    QPixmap getCaptureImage();
    void setIdle();
    void setActive();
    static QStringList getCameraNames();
private:
    QCamera* mCamera;
    QCameraImageCapture *mCapture;
    CameraGenericSurface *mSurface;
    QPixmap mLastImage;
signals:
    void newImageCaptured(void);
private slots:
    void imageCaptured(int id, QImage img);
    void readyForCaptureChanged(bool state);
};


#endif //_CAMERA_GENERIC_H
