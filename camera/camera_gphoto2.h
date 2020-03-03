#ifndef _CAMERA_GPHOTO2_H
#define _CAMERA_GPHOTO2_h
#include "camera_interface.h"
#include <QPixmap>
#include <QMap>
#include <gphoto2/gphoto2.h>

class CameraGphoto2 : public CameraInterface {
public:
    CameraGphoto2();
    ~CameraGphoto2();
    bool initCamera();
    QPixmap getPreviewImage();
    QPixmap getCaptureImage();
    void setIdle();
    void setActive();
    bool setConfigValueString(QString key, QString val);
    QMap<QString, QVariant> getConfig();
    QVariant getConfig(QString key);
private:
    bool mActive;
    GPContext *mContext;
    Camera *mCamera;
    void setStartupConfig();
    void setShutdownConfig();
    QMap<QString, QVariant> getConfigRecursively(CameraWidget *widget);
    QMap<QString, QVariant> getChildValue(CameraWidget *child);
    QMap<QString, QVariant> mStartupConfig;
    QMap<QString, QVariant> mShutdownConfig;
    QMap<QString, QVariant> mIdleConfig;
    QMap<QString, QVariant> mActiveConfig;
};

#endif //_CAMERA_GPHOTO2_H
