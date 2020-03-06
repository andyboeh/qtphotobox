#ifndef _PRINTER_SELPHY
#define _PRINTER_SELPHY

#include "printer_interface.h"

class printerSelphy : public printerInterface {
public:
    printerSelphy();
    ~printerSelphy();
    bool printImage(QPixmap image, int numcopies);
    bool initPrinter();
    QString getStatus();
};

#endif //_PRINTER_SELPHY
