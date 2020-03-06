#ifndef _PRINTER_INTERFACE_H
#define _PRINTER_INTERFACE_H

#include <QString>
#include <QPixmap>

class printerInterface {
public:
    printerInterface();
    virtual ~printerInterface();
    virtual bool initPrinter();
    virtual bool printImage(QPixmap image, int numcopies);
    virtual QString getStatus();
};
#endif //_PRINTER_INTERFACE_H
