#ifndef _CAMERA_GENERIC_SURFACE_H
#define _CAMERA_GENERIC_SURFACE_H

#include <QAbstractVideoSurface>
#include <QAbstractVideoBuffer>
#include <QVideoFrame>
#include <QList>
#include <QPixmap>

class CameraGenericSurface : public QAbstractVideoSurface {
    Q_OBJECT
public:
    CameraGenericSurface(QObject *parent = NULL);
    ~CameraGenericSurface();
    QList<QVideoFrame::PixelFormat> supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const;

    bool present(const QVideoFrame& frame);

    QPixmap getCurrentFrame();
private:
    QPixmap mCurrentFrame;
};


#endif //_CAMERA_GENERIC_SURFACE_H
