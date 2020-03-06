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

bool printerInterface::printImage(QPixmap image, int numcopies)
{
    return false;
}

QString printerInterface::getStatus()
{
    return "Invalid Interface.";
}
