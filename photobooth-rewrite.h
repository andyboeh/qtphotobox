#ifndef _PHOTOBOOTH_REWRITE_H
#define _PHOTOBOOTH_REWRITE_H

#include <QMainWindow>
#include <QPixmap>

class QThread;
class pbCamera;
class pictureWorker;
class postprocessWorker;
class printerWorker;
class showWorker;
class gpioWorker;

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
    void cameraError(QString error);
    void genericError(QString error);
    void startPrintJob(int numcopies);
    void thumbnailScaled(QString path);
    void fullImageSaved(QString filename, bool ret);
    void assembledImageSaved(QString filename, bool ret);
signals:
    void stopCameraThread();
    void stopPictureWorkerThread();
    void stopPostprocessWorkerThread();
    void stopPrinterThread();
    void stopShowThread();
    void stopGpioThread();
    void initAssembleTask();
    void finishTask();
    void addPicture(QPixmap image);
    void saveFullPicture(QPixmap image);
    void saveAssembledPicture(QPixmap image);
    void saveThumbnail(QString name);
    void printPicture(QPixmap image, int copies);
    void initPrinter();
    void retryOperation();
private slots:
    void keyPressEvent(QKeyEvent *event);
    void errorOk();
    void errorQuit();
    void errorRetry();
private:
    QWidget *mCurrentWidget;
    QWidget *mOverlayWidget;
    QThread *mCameraThread;
    QThread *mPictureWorkerThread;
    QThread *mPostprocessWorkerThread;
    QThread *mPrinterThread;
    QThread *mShowThread;
    QThread *mGpioThread;
    showWorker *mShowThreadObject;
    pbCamera *mCameraThreadObject;
    pictureWorker *mPictureWorkerThreadObject;
    postprocessWorker *mPostprocessWorkerThreadObject;
    printerWorker *mPrinterThreadObject;
    gpioWorker *mGpioThreadObject;
    int mImagesCaptured;
    int mImagesToCapture;
    QPixmap mImageToReview;
    bool mErrorPresent;
    void initThreads();
};

#endif //_PHOTOBOOTH_REWRITE_H
