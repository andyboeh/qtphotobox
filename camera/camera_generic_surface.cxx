#include "camera_generic_surface.h"
#include <QDebug>

CameraGenericSurface::CameraGenericSurface(QObject *parent) : QAbstractVideoSurface(parent)
{

}

CameraGenericSurface::~CameraGenericSurface()
{

}

QList<QVideoFrame::PixelFormat> CameraGenericSurface::supportedPixelFormats(QAbstractVideoBuffer::HandleType type) const
{
    return QList<QVideoFrame::PixelFormat>() << QVideoFrame::Format_RGB24 << QVideoFrame::Format_RGB32;
}

bool CameraGenericSurface::present(const QVideoFrame &frame)
{
    if(frame.isValid()) {
        QVideoFrame cloneFrame(frame);
        cloneFrame.map(QAbstractVideoBuffer::ReadOnly);
        const QImage img(cloneFrame.bits(), cloneFrame.width(), cloneFrame.height(),
                         QVideoFrame::imageFormatFromPixelFormat(cloneFrame.pixelFormat()));
        mCurrentFrame = QPixmap::fromImage(img.convertToFormat(QImage::Format_RGB888));
        cloneFrame.unmap();
        return true;
    }
    return false;
}

QPixmap CameraGenericSurface::getCurrentFrame()
{
    qDebug() << mCurrentFrame;
    return mCurrentFrame;
}
