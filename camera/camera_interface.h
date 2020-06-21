#ifndef _CAMERA_INTERFACE_H
#define _CAMERA_INTERFACE_H

#include <QPixmap>
#include <QObject>

class CameraInterface : public QObject{
    Q_OBJECT
public:
    CameraInterface();
    virtual ~CameraInterface();
    virtual bool initCamera();
    virtual QPixmap getPreviewImage();
    virtual QPixmap getCaptureImage();
    virtual void setActive();
    virtual void setIdle();
    virtual void setCameraName(QString name);
protected:
    QString mCameraName;
};

#endif //_CAMERA_INTERFACE_H
