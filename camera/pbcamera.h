#ifndef _PBCAMERA_H
#define _PBCAMERA_H

#include "worker.h"
#include <QPixmap>

class CameraInterface;
class QTimer;

class pbCamera : public Worker
{
    Q_OBJECT
public:
    pbCamera();
    ~pbCamera();
public slots:
    void initCamera(void);
    void start(void);
    void stop(void);
    void startPreview(void);
    void stopPreview(void);
    void captureImage(void);
    void retryOperation();
signals:
    void cameraInitialized(bool ret);
    void previewImageCaptured(QPixmap image);
    void imageCaptured(QPixmap image);
    void cameraError(QString message);
private:
    CameraInterface* mCamera;
    bool mLimitFps;
    QTimer *mLimitTimer;
};

#endif //_PBCAMERA_H
