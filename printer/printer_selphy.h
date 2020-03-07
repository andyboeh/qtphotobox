#ifndef _PRINTER_SELPHY
#define _PRINTER_SELPHY

#include "printer_interface.h"
#include <QByteArray>

class QTcpSocket;
class QUdpSocket;

class printerSelphy : public printerInterface {
    typedef enum {
        CPNP_MSG_DISCOVER = 0x101,
        CPNP_MSG_STARTTCP = 0x110,
        CPNP_MSG_ID       = 0x130,
        CPNP_MSG_STATUS   = 0x120,
        CPNP_MSG_DATA     = 0x121,
        CPNP_MSG_FLUSH    = 0x151
    } selphyCommands;
public:
    printerSelphy();
    ~printerSelphy();
    bool printImage(QPixmap image, int numcopies);
    bool initPrinter();
    QString getStatus();
private:
    QTcpSocket *mTcpSocket;
    QUdpSocket *mUdpSocket;
    QByteArray makePacket(qint32 command, QByteArray payload, qint32 options = 0);
    qint32 mSeqCount;
    QByteArray exchangePackets(QByteArray data);
    bool flush();
};

#endif //_PRINTER_SELPHY
