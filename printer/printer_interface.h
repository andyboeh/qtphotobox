#ifndef _PRINTER_INTERFACE_H
#define _PRINTER_INTERFACE_H

#include <QString>
#include <QPixmap>
#include <QObject>

class printerInterface : public QObject {
    Q_OBJECT
public:
    printerInterface();
    virtual ~printerInterface();
    virtual bool initPrinter();
    virtual bool canPrintFiles();
    virtual bool printFile(QString filename, int numcopies);
    virtual bool printImage(QPixmap image, int numcopies);
    virtual QString getStatus();
signals:
    void printerError(QString err_code, QString error);
};
#endif //_PRINTER_INTERFACE_H
