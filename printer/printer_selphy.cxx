#include "printer_selphy.h"
#include <QDebug>

printerSelphy::printerSelphy()
{

}

printerSelphy::~printerSelphy()
{

}

bool printerSelphy::printImage(QPixmap image, int numcopies)
{
    qDebug() << "printerSelphy::printImage";
    return false;
}

bool printerSelphy::initPrinter()
{
    qDebug() << "printerSelphy::initPrinter";
    return false;
}

QString printerSelphy::getStatus()
{
    qDebug() << "printerSelphy::getStatus";
    return "";
}
