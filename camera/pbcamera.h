#ifndef _PBCAMERA_H
#define _PBCAMERA_H

#include "worker.h"
#include <QPixmap>

class CameraInterface;

class pbCamera : public Worker
{
    Q_OBJECT
public:
    pbCamera();
    ~pbCamera();
public slots:
    bool initCamera(void);
    void start(void);
    void stop(void);
    void startPreview(void);
    void stopPreview(void);
    void captureImage(void);
signals:
    void cameraInitialized(bool ret);
    void finished();
    void previewImageCaptured(QPixmap image);
    void imageCaptured(QPixmap image);
private:
    QString mCommand;
    CameraInterface* mCamera;
    bool checkForNewCommand();
};

#endif //_PBCAMERA_H
