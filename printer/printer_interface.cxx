#include "printer_interface.h"

printerInterface::printerInterface()
{

}

printerInterface::~printerInterface()
{

}

bool printerInterface::initPrinter()
{
    return false;
}

bool printerInterface::canPrintFiles()
{
    return false;
}

bool printerInterface::printFile(QString filename, int numcopies)
{
    return false;
}

bool printerInterface::printImage(QPixmap image, int numcopies)
{
    return false;
}

QString printerInterface::getStatus()
{
    return "Invalid Interface.";
}
