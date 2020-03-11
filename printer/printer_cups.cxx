#include "printer_cups.h"
#include <cups/cups.h>
#include <QDebug>
#include <QTemporaryFile>

printerCups::printerCups()
{

}

printerCups::~printerCups()
{

}

bool printerCups::printImage(QPixmap image, int numcopies)
{
    QTemporaryFile saveFile;
    int num_options;
    cups_option_t *options;

    num_options = 0;
    options = NULL;

    if(!saveFile.open()) {
        qDebug() << "Error opening temporary file.";
        return false;
    }

    if(!image.save(&saveFile, "JPG")) {
        qDebug() << "Error saving to temporary file.";
        return false;
    }

    saveFile.close();

    num_options = cupsAddOption("copies", QString::number(numcopies).toStdString().c_str(), num_options, &options);
    const char *name = cupsGetDefault();
    int jobid = cupsPrintFile(name, saveFile.fileName().toStdString().c_str(), "QtPhotobox", num_options, options);
    if(jobid < 1) {
        qDebug() << "Error submitting print job.";
        return false;
    }
    return true;
}

bool printerCups::initPrinter()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);

    if(num_dests == 0) {
        qDebug() << "No printer found.";
        return false;
    }

    if(getStatus() == "ok")
        return true;

    return false;
}

QString printerCups::getStatus()
{
    cups_dest_t *dests;
    int num_dests = cupsGetDests(&dests);

    QString stateReason = "unknown";
    int state = -1;

    for(int i=0; i<num_dests; i++) {
        if(dests[i].is_default) {
            qDebug() << "Found default printer: " << dests[i].name;
            for(int j=0; j<dests[i].num_options; j++) {
                QString key(dests[i].options[j].name);
                if(key == "printer-state-reasons") {
                    stateReason = dests[i].options[j].value;
                } else if(key == "printer-state") {
                    state = QString(dests[i].options[j].value).toInt();
                }
            }
        }
    }

    if(state == 3 || state == 4)
        return "ok";

    qDebug() << "Printer state: " << state << ": " << stateReason;
    return stateReason;
}
