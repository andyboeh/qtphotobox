#ifndef _PRINTER_CUPS
#define _PRINTER_CUPS

#include "printer_interface.h"
#include <QPixmap>
#include <QString>

class printerCups : public printerInterface {
public:
    printerCups();
    ~printerCups();
    bool printImage(QPixmap image, int numcopies);
    bool initPrinter();
    QString getStatus();
};

#endif //_PRINTER_CUPS
