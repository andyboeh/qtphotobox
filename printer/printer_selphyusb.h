#ifndef _PRINTER_SELPHYUSB_H
#define _PRINTER_SELPHYUSB_H

#include "printer_interface.h"
#include "printJob.h"
#include <QList>
#include <QMap>
#include <QString>
#include <QByteArray>

struct libusb_device_handle;
struct libusb_context;
class QTimer;

class printerSelphyUsb : public printerInterface {
    Q_OBJECT
    typedef enum {
        JOB_STATE_IDLE,
        JOB_STATE_DATA_SENT,
        JOB_STATE_DONE,
        JOB_STATE_ERROR
    } selphyUsbJobState;
public:
    printerSelphyUsb();
    ~printerSelphyUsb();
    bool canPrintFiles();
    bool printImage(QPixmap image, int numcopies);
    bool initPrinter();
    bool printFile(QString filename, int numcopies);
    QString getStatus(bool readTwice = false);
private slots:
    void processPrintJob();
private:
    QMap<quint16, QString> mPids;
    quint16 mVid;
    libusb_device_handle *mDev;
    libusb_context *mCtx;
    uint8_t mEndpointUp;
    uint8_t mEndpointDown;
    uint8_t mInterface;
    uint8_t mAltSetting;
    int mXferTimeout;
    QList<printJob> mPrintJobs;
    QTimer *mTimer;
    selphyUsbJobState mJobState;
    bool mErrorState;

    QByteArray readData();
    bool writeData(const QByteArray data);
    QString parseError(uint8_t err);
    bool resetStatus();
    bool prepareSpoolData(QPixmap image, int numcopies);
    QStringList getStatusAndError();
    QString parseStatus(uint8_t status);
};


#endif //_PRINTER_SELPHYUSB_H
