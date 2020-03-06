#ifndef _PHOTOBOOTH_REWRITE_H
#define _PHOTOBOOTH_REWRITE_H

#include <QMainWindow>
#include <QPixmap>

class QThread;
class pbCamera;
class pictureWorker;
class postprocessWorker;
class printerThreadObject;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
public slots:
    void changeState(QString name);
    void loadSettingsToGui(bool showWindow);
    void imageCaptured(QPixmap image);
    void pictureAssembled(QPixmap image);
    void printerError(QString error);
    void startPrintJob(int numcopies);
signals:
    void stopCameraThread();
    void stopPictureWorkerThread();
    void stopPostprocessWorkerThread();
    void stopPrinterThread();
    void initAssembleTask();
    void finishTask();
    void addPicture(QPixmap image);
    void saveFullPicture(QPixmap image);
    void saveAssembledPicture(QPixmap image);
    void printPicture(QPixmap image, int copies);
    void initPrinter();
private:
    QWidget *mCurrentWidget;
    QWidget *mOverlayWidget;
    QThread *mCameraThread;
    QThread *mPictureWorkerThread;
    QThread *mPostprocessWorkerThread;
    QThread *mPrinterThread;
    pbCamera *mCameraThreadObject;
    pictureWorker *mPictureWorkerThreadObject;
    postprocessWorker *mPostprocessWorkerThreadObject;
    printerThreadObject *mPrinterThreadObject;
    int mImagesCaptured;
    int mImagesToCapture;
    QPixmap mImageToReview;
    bool mFullscreen;
};

#endif //_PHOTOBOOTH_REWRITE_H
