#include "printer.h"
#ifdef BUILD_SELPHY_WIFI
#include "printer_selphy.h"
#endif
#ifdef BUILD_CUPS
#include "printer_cups.h"
#endif
#ifdef BUILD_SELPHY_USB
#include "printer_selphyusb.h"
#endif
#include "printJob.h"
#include "settings.h"
#include <QThread>
#include <QTimer>

printerWorker::printerWorker()
{
    mPrinter = nullptr;
    mTimer = new QTimer();
    mTimer->setInterval(5000);
    mTimer->setSingleShot(false);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(startStatusPolling()));
}

printerWorker::~printerWorker()
{
    if(mTimer) {
        mTimer->stop();
        mTimer->deleteLater();
        mTimer = nullptr;
    }
    if(mPrinter) {
        delete mPrinter;
        mPrinter = nullptr;
    }
}

void printerWorker::printerErrorInternal(QString err_code, QString error) {
    emit printerError(error);
}

void printerWorker::start()
{
    pbSettings &pbs = pbSettings::getInstance();
    QString backend = pbs.get("printer", "backend");
#ifdef BUILD_SELPHY_WIFI
    if(backend == "selphy") {
        mPrinter = new printerSelphy();
    }
#endif
#ifdef BUILD_CUPS
    if(backend == "cups") {
        mPrinter = new printerCups();
    }
#endif
#ifdef BUILD_SELPHY_USB
    if(backend == "selphyusb") {
        mPrinter = new printerSelphyUsb();
    }
#endif
    if(!mPrinter) {
        emit printerError(tr("Printer backend not found or unknown."));
        emit finished();
        return;
    }
    connect(mPrinter, SIGNAL(printerError(QString,QString)), this, SLOT(printerErrorInternal(QString,QString)));

    bool running = true;
    bool initialized = false;

    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            return;

        QString command = mCommandList.takeFirst();

        if(command == "processJob") {
            if(mPrintJobs.isEmpty())
                continue;

            if(!initialized) {
                mCommandList.prepend("initPrinter");
                continue;
            }
            printJob job = mPrintJobs.takeFirst();
            if(mPrinter) {
                if(job.isFileJob()) {
                    mPrinter->printFile(job.getFile(), job.getCopies());
                } else {
                    mPrinter->printImage(job.getImage(), job.getCopies());
                }
            }
        } else if(command == "processJobs") {
            int count = mPrintJobs.size();
            for(int i=0; i<count; i++)
                mCommandList.append("processJob");
        } else if(command == "initPrinter") {
            if(!mPrinter->initPrinter())
                emit printerError(tr("Error initializing printer. Is it turned on?"));
            else {
                initialized = true;
                if(mCommandList.isEmpty() && !mPrintJobs.isEmpty()) {
                    mCommandList.append("processJobs");
                }
            }
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
    mTimer->deleteLater();
    mTimer = nullptr;
    delete mPrinter;
    mPrinter = nullptr;
    emit finished();
}

void printerWorker::stop()
{
    qDebug() << "printerWorker::stop";
    mCommandList.append("stopThread");
}

void printerWorker::startStatusPolling()
{
    mCommandList.append("startStatusPolling");
}

void printerWorker::stopStatusPolling()
{
    mCommandList.append("stopStatusPolling");
}

void printerWorker::addPrintJob(QPixmap image, int numcopies)
{
    printJob job(image, numcopies);
    mPrintJobs.append(job);
    mCommandList.append("processJob");
}

void printerWorker::addFilePrintJob(QString filename, int numcopies)
{
    if(mPrinter->canPrintFiles())
    {
        printJob job(filename, numcopies);
        mPrintJobs.append(job);
        mCommandList.append("processJob");
    } else {
        QPixmap image(filename);
        addPrintJob(image, numcopies);
    }
}

void printerWorker::initPrinter()
{
    mCommandList.append("initPrinter");
}
