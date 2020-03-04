#ifndef _PHOTOBOOTH_REWRITE_H
#define _PHOTOBOOTH_REWRITE_H

#include <QMainWindow>

class QThread;
class pbCamera;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
public slots:
    void changeState(QString name);
    void loadSettingsToGui(void);
    void imageCaptured(void);
signals:
    void stopCameraThread();
private:
    QWidget* mCurrentWidget;
    QThread* mCameraThread;
    pbCamera* mCameraThreadObject;
    int mImagesCaptured;
    int mImagesToCapture;
};

#endif //_PHOTOBOOTH_REWRITE_H
