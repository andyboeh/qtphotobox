#ifndef _PRINTER_H
#define _PRINTER_H

#include "worker.h"
#include <QPixmap>

class printJob;
class printerInterface;
class QTimer;

class printerThreadObject : public Worker
{
    Q_OBJECT
public:
    printerThreadObject();
    ~printerThreadObject();
public slots:
    void start(void);
    void stop(void);
    void startStatusPolling(void);
    void stopStatusPolling(void);
    void addPrintJob(QPixmap image, int numcopies);
    void initPrinter(void);
signals:
    void printerError(QString error);
private:
    printerInterface* mPrinter;
    QTimer *mTimer;
    QList<printJob> mPrintJobs;
};

#endif //_PBCAMERA_H
