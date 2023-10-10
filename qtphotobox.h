#ifndef _QTPHOTOBOX_H
#define _QTPHOTOBOX_H

#include <QMainWindow>
#include <QPixmap>
#include <QTranslator>

class QThread;
class pbCamera;
class pictureWorker;
class postprocessWorker;
class printerWorker;
class showWorker;
#ifdef BUILD_PIGPIO
class gpioWorker;
#endif
class screenSaver;
#ifdef BUILD_CURL
class curlWorker;
#endif

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
    void thumbnailScaled(QString path, QString filename);
    void fullImageSaved(QString path, QString filename, bool ret);
    void assembledImageSaved(QString path, QString filename, bool ret);
    void removableDeviceDetected(QString path);
signals:
    void stopCameraThread();
    void stopPictureWorkerThread();
    void stopPostprocessWorkerThread();
    void stopPrinterThread();
    void stopCurlThread();
    void stopShowThread();
    void stopGpioThread();
    void initAssembleTask();
    void finishTask();
    void addPicture(QPixmap image);
    void saveFullPicture(QPixmap image);
    void saveAssembledPicture(QPixmap image);
    void saveThumbnail(QString name);
    void printPicture(QPixmap image, int copies);
    void printPictureFile(QString filename, int copies);
    void initPrinter();
    void retryOperation();
    void addPictureToShow(QString path);
    void initializeCamera();
    void waitForRemovableDevice();
    void sendAttachmentViaEmail(QString path);
private slots:
    void keyPressEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);
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
#ifdef BUILD_PIGPIO
    QThread *mGpioThread;
#endif
#ifdef BUILD_CURL
    QThread *mCurlWorkerThread;
#endif
    QTranslator mTranslator;
    showWorker *mShowThreadObject;
    pbCamera *mCameraThreadObject;
    pictureWorker *mPictureWorkerThreadObject;
    postprocessWorker *mPostprocessWorkerThreadObject;
    printerWorker *mPrinterThreadObject;
#ifdef BUILD_PIGPIO
    gpioWorker *mGpioThreadObject;
#endif
    screenSaver *mScreenSaver;
#ifdef BUILD_CURL
    curlWorker *mCurlWorkerObject;
#endif
    int mImagesCaptured;
    int mImagesToCapture;
    QPixmap mImageToReview;
    bool mErrorPresent;
    QString mImageToPrint;
    void initThreads();
    QString mStateBeforeError;
};

#endif //_QTPHOTOBOX_H
