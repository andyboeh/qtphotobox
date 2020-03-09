#include "printer_selphy.h"
#include "settings.h"
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <QTimer>
#include <QtEndian>

printerSelphy::printerSelphy()
{
    mTcpSocket = new QTcpSocket();
    mUdpSocket = new QUdpSocket();
    mSeqCount = 0;
    connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(processTcpData()));
    mJobState = JOB_STATE_IDLE;
    mCurrentFile = nullptr;
    QTimer *mTimer = new QTimer();
    mTimer->setSingleShot(false);
    mTimer->setInterval(1000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(processPrintJob()));
}

printerSelphy::~printerSelphy()
{
    if(mCurrentFile) {
        mCurrentFile->close();
    }
    delete mCurrentFile;
    mCurrentFile = nullptr;
    delete mTcpSocket;
    mTcpSocket = nullptr;
    delete mUdpSocket;
    mUdpSocket = nullptr;
    mTimer->stop();
    delete mTimer;
    mTimer = nullptr;
}

bool printerSelphy::canPrintFiles()
{
    return true;
}

QByteArray printerSelphy::makePacket(qint16 command, QByteArray payload, qint32 options) {
    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds.writeRawData("CPNP", 4);
    ds << command;
    ds << mSeqCount;
    ds << options;
    ds << (qint16)payload.size();
    if(payload.size() > 0) {
        packet.append(payload);
    }
    mSeqCount++;
    return packet;
}

bool printerSelphy::printImage(QPixmap image, int numcopies)
{
    qDebug() << "printerSelphy::printImage";

    return false;
}

void printerSelphy::processTcpData() {
    mData.append(mTcpSocket->readAll());

    if(mData.size() < 16) {
        qDebug() << "TCP Response Packet too short.";
        return;
    }
    int payloadLength = mData.mid(14, 2).toInt();
    if(mData.size() < 16 + payloadLength) {
        qDebug() << "TCP Payload not fully received" << payloadLength << " vs. " << (mData.size() - 16);
        return;
    }

    processPrintJob();
    mData.clear();
}

QByteArray printerSelphy::exchangePackets(QByteArray data)
{
    int timeout = 5 * 1000;
    mUdpSocket->write(data);
    /*
    if(!mUdpSocket->waitForBytesWritten(timeout)) {
        qDebug() << "Timed out connecting to printer";
        return QByteArray();
    }
    */
    if(!mUdpSocket->waitForReadyRead(timeout)) {
        qDebug() << "Timed out waiting for response from printer.";
        return QByteArray();
    }
    QByteArray response = mUdpSocket->readAll();
    qDebug() << response;
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

bool printerSelphy::connectUdp(bool reconnect) {
    int timeout = 5*1000;

    if(mUdpSocket->isValid() && reconnect) {
        mUdpSocket->disconnectFromHost();
        mSeqCount = 1;
    }
    mUdpSocket->connectToHost(mIp, mPort);

    if(!mUdpSocket->waitForConnected(timeout)) {
        qDebug() << "Error connecting to printer";
        return false;
    }
    return true;
}

bool printerSelphy::connectTcp(bool reconnect) {
    int timeout = 5*1000;

    if(mTcpSocket->state() == QTcpSocket::ConnectedState && reconnect) {
        mTcpSocket->disconnectFromHost();
    }
    mTcpSocket->connectToHost(mIp, mTcpPort);
    if(!mTcpSocket->waitForConnected(timeout)) {
        qDebug() << "Error connecting to printer via TCP";
        return false;
    }
    return true;

}

printerSelphy::selphyState printerSelphy::getSelphyState() {

    QByteArray data = makePacket(CPNP_MSG_STATUS, QByteArray());
    QByteArray response = exchangePackets(data);
    if(response.isEmpty()) {
        qDebug() << "Timed out exchanging data with printer.";
        return SELPHY_OFFLINE;
    }

    QByteArray body = response.mid(16);
    if(body.size() < 4) {
        qDebug() << "Error: body too short: " << body.size();
        return SELPHY_OFFLINE;
    }
    bool paper = (body.at(2) != 0x01);
    bool ribbon = (body.at(3) != 0x01);

    if(!ribbon && !paper)
        return SELPHY_RIBBON_AND_PAPER_NOT_INSTALLED;
    if(!ribbon)
        return SELPHY_RIBBON_NOT_INSTALLED;
    if(!paper)
        return SELPHY_PAPER_NOT_INSTALLED;

    return SELPHY_OK;
}

bool printerSelphy::initPrinter()
{

    qDebug() << "printerSelphy::initPrinter";
    pbSettings &pbs = pbSettings::getInstance();
    QString ip = pbs.get("printer", "ip");
    int port = 8609;
    mIp = ip;
    mPort = port;

    if(!connectUdp(true)) {
        return false;
    }

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

    selphyState state = getSelphyState();
    if(state != SELPHY_OFFLINE) {
        flush();
        mTimer->start();
        return true;
    }


    return false;
}

void printerSelphy::processPrintJob(bool tcp) {
    if(!tcp) {
        if(mPrintJobs.isEmpty())
            return;
        if(mJobState != JOB_STATE_IDLE)
            return;
    }

    switch(mJobState) {
    case JOB_STATE_IDLE:
    {
        if(mPrintJobs.isEmpty())
            return;
        printJob job = mPrintJobs.first();
        if(mCurrentFile) {
            mCurrentFile->close();
            delete mCurrentFile;
            mCurrentFile = nullptr;
        }
        mCurrentFile = new QFile(job.getFile());
        mCurrentFile->open(QIODevice::ReadOnly);
        QByteArray payload(0x188, '\x0');
        QByteArray un = "pb_selphy";
        QByteArray fn = mCurrentFile->fileName().toStdString().c_str();
        un.truncate(0x40);
        fn.truncate(0x100);
        payload = payload.replace(0x008, un.size(), un);
        payload = payload.replace(0x048, un.size(), un);
        payload = payload.replace(0x088, fn.size(), fn);

        QByteArray data = makePacket(CPNP_MSG_STARTTCP, payload);
        QByteArray response = exchangePackets(data);
        mTcpPort = response.mid(20, 2).toInt();
        mJobSeq = response.mid(10,6);
        if(mTcpPort == 0) {
            qDebug() << "Printer not ready to accept data!";
            mJobState = JOB_STATE_IDLE;
            return;
        }
        if(!connectTcp(true)) {
            mJobState = JOB_STATE_IDLE;
            return;
        }
        data = makePacket(CPNP_MSG_STATUS, QByteArray());
        mJobState = JOB_STATE_TCP_STARTED;
        mTcpSocket->write(data);
    }
    case JOB_STATE_TCP_STARTED:
    {
        if(mData == mLastData) {
            QTimer::singleShot(500, this, "pollTcpDelayed");
            return;
        }
        int state = mData.mid(16 + 18).toInt();
        switch(state) {
        case 0x00:
            qDebug() << "Wait";
            QTimer::singleShot(500, this, "pollTcpDelayed");
            break;
        case 0x01:
            qDebug() << "Send flags";
            mJobState = JOB_STATE_FLAGS_SENT;
            sendFlags();
            break;
        case 0x02:
        {
            qDebug() << "File data request";
            quint32 offset = qFromLittleEndian<quint32>(mData.mid(16+0x18,4).constData());
            quint32 length = qFromLittleEndian<quint32>(mData.mid(16+0x1c,4).constData());
            qDebug() << "Will send " << length << " bytes starting from " << offset;
            mCurrentFile->seek(offset);
            mFileData = mCurrentFile->read(length);
            sendChunk();
            break;
        }
        case 0x03:
            qDebug() << "Job done.";
            sendJobDone();
            mJobState = JOB_STATE_DONE;
            break;
        case 0x04:
            qDebug() << "Printer error.";
            mJobState = JOB_STATE_FAILED;
            break;
        default:
            qDebug() << "Printer reported unknown state.";
            mJobState = JOB_STATE_FAILED;
        }

        break;
    }
    case JOB_STATE_FLAGS_SENT:
        qDebug() << "Poll";
        mJobState = JOB_STATE_TCP_STARTED;
        pollTcpDelayed();
        break;
    case JOB_STATE_CHUNK_SENT:
        qDebug() << "Chunk sent";
        if(mFileData.size() > 0) {
            sendChunk();
        } else {
            mCurrentFile->close();
            mJobState = JOB_STATE_TCP_STARTED;
            pollTcpDelayed();
        }
        break;
    case JOB_STATE_DONE:
        mPrintJobs.removeFirst();
        mJobState = JOB_STATE_IDLE;
        break;
    case JOB_STATE_FAILED:
        mJobState = JOB_STATE_IDLE;
        break;
    }
}

void printerSelphy::sendJobDone() {
    QByteArray payload(0x40, '\x0');
    const quint32 bs = qToLittleEndian<quint32>(payload.size());
    payload.replace(0x04, 4, (const char *)&bs);
    payload.replace(2, 1, "\x3");
    QByteArray data = makePacket(CPNP_MSG_DATA, payload);
    mJobState = JOB_STATE_DONE;
    mTcpSocket->write(data);
}

void printerSelphy::sendChunk() {
    int size = mFileData.size();
    if(size > 4096) {
        size = 4096;
    }
    QByteArray payload = mFileData.mid(0, size);
    mFileData = mFileData.remove(0, 4096);
    QByteArray data = makePacket(CPNP_MSG_DATA, payload);
    mJobState = JOB_STATE_CHUNK_SENT;
    mTcpSocket->write(data);
}

void printerSelphy::sendFlags() {
    QByteArray payload(0x40, '\x0');
    const quint32 bs = qToLittleEndian<quint32>(payload.size());
    const quint32 one = qToLittleEndian<quint32>(1);
    payload.replace(0x04, 4, (const char *)&bs);
    payload.replace(0x0c,4, (const char*)&one);
    // No border: 2
    // Allow border: 3
    const quint32 crop = qToLittleEndian<quint32>(2);
    payload.replace(0x12, 4, (const char*)&crop);

    QByteArray data = makePacket(CPNP_MSG_DATA, payload);
    mTcpSocket->write(data);
}

void printerSelphy::pollTcpDelayed()
{
    QByteArray data = makePacket(CPNP_MSG_STATUS, QByteArray());
    mTcpSocket->write(data);
}

bool printerSelphy::printFile(QString filename, int numcopies)
{
    qDebug() << "printerSelphy::printFile";
    printJob job(filename, numcopies);
    mPrintJobs.append(job);
    return true;
}

QString printerSelphy::getStatus()
{
    QString ret = "unknown";
    qDebug() << "printerSelphy::getStatus";
    if(!connectUdp(false)) {
        return "offline";
    }
    selphyState state = getSelphyState();
    switch(state) {
    case SELPHY_OFFLINE:
        ret = "offline";
        break;
    case SELPHY_PAPER_NOT_INSTALLED:
        ret = "no-paper";
        break;
    case SELPHY_RIBBON_NOT_INSTALLED:
        ret = "no-ribbon";
        break;
    case SELPHY_RIBBON_AND_PAPER_NOT_INSTALLED:
        ret = "no-ribbon-and-paper";
        break;
    case SELPHY_OK:
        ret = "ok";
        break;
    }

    return ret;
}
