#include "printer.h"
#include "printer_selphy.h"
#include "printer_cups.h"
#include "printJob.h"
#include "settings.h"
#include <QThread>
#include <QTimer>

printerThreadObject::printerThreadObject()
{
    mPrinter = nullptr;
    mTimer = new QTimer();
    mTimer->setInterval(5000);
    mTimer->setSingleShot(false);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(startStatusPolling()));
}

printerThreadObject::~printerThreadObject()
{
    if(mTimer)
        mTimer->stop();
    delete mTimer;
    mTimer = nullptr;
    delete mPrinter;
    mPrinter = nullptr;
}

void printerThreadObject::start()
{
    pbSettings &pbs = pbSettings::getInstance();
    QString backend = pbs.get("printer", "backend");
    if(backend == "selphy") {
        mPrinter = new printerSelphy();
    } else if(backend == "cups") {
        mPrinter = new printerCups();
    }

    bool running = true;

    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            return;

        QString command = mCommandList.takeFirst();

        if(command == "processJob") {
            if(mPrintJobs.isEmpty())
                continue;

            printJob job = mPrintJobs.takeFirst();
            if(mPrinter) {
                mPrinter->printImage(job.getImage(), job.getCopies());
            }
        } else if(command == "initPrinter") {
            mPrinter->initPrinter();
        } else if(command == "startStatusPolling") {
            if(!mTimer->isActive()) {
                mTimer->start();
            }
            QString status = mPrinter->getStatus();
            if(status != "ok") {
                emit printerError(status);
            }
        } else if(command == "stopStatusPolling") {
            mTimer->stop();
        } else if(command == "stopThread") {
            qDebug() << "stopThread in printerThread";
            running = false;
        }
    }

    mTimer->stop();
    delete mTimer;
    mTimer = nullptr;
    delete mPrinter;
    mPrinter = nullptr;
    emit finished();
}

void printerThreadObject::stop()
{
    qDebug() << "printerThreadObject::stop";
    mCommandList.append("stopThread");
}

void printerThreadObject::startStatusPolling()
{
    mCommandList.append("startStatusPolling");
}

void printerThreadObject::stopStatusPolling()
{
    mCommandList.append("stopStatusPolling");
}

void printerThreadObject::addPrintJob(QPixmap image, int numcopies)
{
    printJob job(image, numcopies);
    mPrintJobs.append(job);
    mCommandList.append("processJob");
}

void printerThreadObject::addFilePrintJob(QString filename, int numcopies)
{
    QPixmap image(filename);
    addPrintJob(image, numcopies);
}

void printerThreadObject::initPrinter()
{
    mCommandList.append("initPrinter");
}
