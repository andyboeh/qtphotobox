#ifndef _PRINTER_H
#define _PRINTER_H

#include "worker.h"
#include <QPixmap>

class printJob;
class printerInterface;
class QTimer;

class printerWorker : public Worker
{
    Q_OBJECT
public:
    printerWorker();
    ~printerWorker();
public slots:
    void start(void);
    void stop(void);
    void startStatusPolling(void);
    void stopStatusPolling(void);
    void addPrintJob(QPixmap image, int numcopies);
    void addFilePrintJob(QString filename, int numcopies);
    void initPrinter(void);
signals:
    void printerError(QString error);
private slots:
    void printerErrorInternal(QString err_code, QString error);
private:
    printerInterface* mPrinter;
    QTimer *mTimer;
    QList<printJob> mPrintJobs;
};

#endif //_PBCAMERA_H
