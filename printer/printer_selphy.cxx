#include "printer_selphy.h"
#include "settings.h"
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>
#include <QDebug>

printerSelphy::printerSelphy()
{
    mTcpSocket = new QTcpSocket();
    mUdpSocket = new QUdpSocket();
    mSeqCount = 0;
}

printerSelphy::~printerSelphy()
{
    delete mTcpSocket;
    mTcpSocket = nullptr;
    delete mUdpSocket;
    mUdpSocket = nullptr;
}

QByteArray printerSelphy::makePacket(qint32 command, QByteArray payload, qint32 options) {
    QByteArray packet;
    packet.append("CPNP");
    packet.append(command);
    packet.append(mSeqCount);
    packet.append(options);
    packet.append((qint16)payload.size());
    if(payload.size() > 0) {
        packet.append(payload);
    }
    return packet;
}

bool printerSelphy::printImage(QPixmap image, int numcopies)
{
    qDebug() << "printerSelphy::printImage";
    return false;
}

QByteArray printerSelphy::exchangePackets(QByteArray data)
{
    int timeout = 5 * 1000;
    mUdpSocket->write(data);
    if(!mUdpSocket->waitForBytesWritten(timeout)) {
        qDebug() << "Timed out connecting to printer";
        return QByteArray();
    }
    if(!mUdpSocket->waitForReadyRead(timeout)) {
        qDebug() << "Timed out waiting for response from printer.";
        return QByteArray();
    }
    QByteArray response = mUdpSocket->readAll();
    qDebug() << response;
    mSeqCount++;
    return response;
}

bool printerSelphy::flush() {
    QByteArray data = makePacket(CPNP_MSG_FLUSH, QByteArray());
    QByteArray response = exchangePackets(data);
    if(response.isEmpty()) {
        qDebug() << "Timed out exchanging data with printer.";
        return false;
    }
    return true;
}

bool printerSelphy::initPrinter()
{
    int timeout = 5*1000;
    qDebug() << "printerSelphy::initPrinter";
    pbSettings &pbs = pbSettings::getInstance();
    QString ip = pbs.get("printer", "ip");
    int port = 8609;
    bool ret;

    if(mUdpSocket->isValid())
        mUdpSocket->disconnectFromHost();
    mUdpSocket->connectToHost(ip, port);
    mSeqCount = 1;
    if(!mUdpSocket->waitForConnected(timeout)) {
        qDebug() << "Error connecting to printer";
        return false;
    }
    /*
    if(mTcpSocket->isOpen())
        mTcpSocket->disconnect();
    mTcpSocket->connectToHost(ip, port);
    mSeqCount = 0;
    if(!mTcpSocket->waitForConnected(timeout)) {
        qDebug() << "Error connecting to printer";
        return false;
    }
    */

    QByteArray data = makePacket(CPNP_MSG_DISCOVER, QByteArray());

    QByteArray response = exchangePackets(data);
    if(response.isEmpty()) {
        qDebug() << "Timed out exchanging data with printer.";
        return false;
    }
    data = makePacket(CPNP_MSG_ID, QByteArray::number((qint32)0));
    response = exchangePackets(data);
    if(response.isEmpty()) {
        qDebug() << "Timed out exchanging data with printer.";
        return false;
    }
    data = makePacket(CPNP_MSG_STATUS, QByteArray());
    response = exchangePackets(data);
    if(response.isEmpty()) {
        qDebug() << "Timed out exchanging data with printer.";
        return false;
    }

    QByteArray body = data.mid(16);
    if(body.at(2) == 0x01) {
        qDebug() << "Paper Cassette not installed.";
    }
    if(body.at(3) == 0x01) {
        qDebug() << "Ribbon not installed.";
    }

    flush();


    return true;
}

QString printerSelphy::getStatus()
{
    qDebug() << "printerSelphy::getStatus";
    return "";
}
