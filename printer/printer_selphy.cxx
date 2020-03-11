#include "printer_selphy.h"
#include "settings.h"
#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QUdpSocket>
#include <QDebug>
#include <QDataStream>
#include <QFile>
#include <QTimer>
#include <QtEndian>
#include <QFileInfo>
#include <QTextCodec>
#include <QTextEncoder>

printerSelphy::printerSelphy()
{
    mTcpSocket = new QTcpSocket();
    mUdpSocket = new QUdpSocket();
    mSeqCount = 1;
    connect(mTcpSocket, SIGNAL(readyRead()), this, SLOT(processTcpData()));
    connect(mTcpSocket, SIGNAL(connected()), this, SLOT(tcpConnected()));
    connect(mTcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(tcpError(QAbstractSocket::SocketError)));
    mJobState = JOB_STATE_IDLE;
    mCurrentFile = nullptr;
    mTimer = new QTimer();
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

void printerSelphy::tcpError(QAbstractSocket::SocketError error) {
    qDebug() << "Received socket error: " << error;
    mTcpSocket->disconnectFromHost();
    if(mJobState == JOB_STATE_DONE) {
        mJobState = JOB_STATE_IDLE;
    } else {
        mJobState = JOB_STATE_FAILED;
    }
}

QByteArray printerSelphy::makePacket(qint16 command, QByteArray payload, qint32 options) {
    QByteArray packet;
    QDataStream ds(&packet, QIODevice::WriteOnly);
    ds.setByteOrder(QDataStream::BigEndian);
    ds.writeRawData("CPNP", 4);
    ds << command;
    ds << mSeqCount;
    ds << options;
    ds << static_cast<quint16>(payload.size());
    if(payload.size() > 0) {
        packet.append(payload);
    }
    //qDebug() << "Payload Size: " << payload.size();
    mSeqCount++;
    return packet;
}

bool printerSelphy::printImage(QPixmap image, int numcopies)
{
    qDebug() << "printerSelphy::printImage";

    return false;
}

void printerSelphy::processTcpData() {
    //qDebug() << "processTcpData";
    mData.append(mTcpSocket->readAll());

    if(mData.size() < 16) {
        qDebug() << "TCP Response Packet too short.";
        return;
    }
    int payloadLength = qFromBigEndian<quint16>(mData.mid(14, 2));
    //qDebug() << "TCP Payload length: " << payloadLength;
    if(mData.size() < 16 + payloadLength) {
        qDebug() << "TCP Payload not fully received" << payloadLength << " vs. " << (mData.size() - 16);
        return;
    }

    processPrintJob(true);
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
    //qDebug() << response.toHex();
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
    //int timeout = 5*1000;

    if(mTcpSocket->state() == QTcpSocket::ConnectedState && reconnect) {
        mTcpSocket->disconnectFromHost();
    }
    mTcpSocket->connectToHost(mIp, mTcpPort);
    /*
    if(!mTcpSocket->waitForConnected(timeout)) {
        qDebug() << "Error connecting to printer via TCP";
        return false;
    }
    qDebug() << "Printer connected.";
    */
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

QByteArray printerSelphy::makeFileHeader(quint32 offset, quint32 length) {
    QByteArray data(0x68, '\x0');
    data = data.replace(0x02, 1, "\x1");
    const quint32 len = qToLittleEndian<quint32>(length + data.size());
    data = data.replace(0x04, 4, (const char*)&len, 4);
    data = data.replace(0x0c, 1, "\x1");
    const quint32 fsize = qToLittleEndian<quint32>(mCurrentFile->size());
    data = data.replace(0x14, 4, (const char*)&fsize, 4);
    const quint32 width = qToLittleEndian<quint32>(mPrintJobs.first().getWidth());
    data = data.replace(0x18, 4, (const char*)&width, 4);
    const quint32 height = qToLittleEndian<quint32>(mPrintJobs.first().getHeight());
    data = data.replace(0x1c, 4, (const char*)&height, 4);
    const quint32 coffset = offset;
    data = data.replace(0x60, 4, (const char*)&coffset, 4);
    const quint32 clength = length;
    data = data.replace(0x64, 4, (const char*)&clength, 4);

    //qDebug() << "makeFileHeader: " << data.toHex();
    return data;
}

QByteArray printerSelphy::bs(QByteArray data) {
    QByteArray out;
    for(int i=1; i<data.size(); i = i+2) {
        out.append(data.at(i));
        out.append(data.at(i-1));
    }
    if(out.size() < data.size())
        out.append(data.at(data.size()-1));
    return out;
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
        if(!mCurrentFile->open(QIODevice::ReadOnly)) {
            qDebug() << "Error opening file.";
        } else {
            qDebug() << "File opened successfully";
        }
        QByteArray payload(0x188, '\x0');

        QTextCodec *codec = QTextCodec::codecForName("UTF-16");
        QTextEncoder *encoderWithoutBom = codec->makeEncoder(QTextCodec::IgnoreHeader);

        QString username("pb_selphy");
        QByteArray un = encoderWithoutBom->fromUnicode(username);

        QString appname("QtPhotobox");
        QByteArray an = encoderWithoutBom->fromUnicode(appname);

        QFileInfo info(job.getFile());

        QString filename = info.fileName();
        QByteArray fn = encoderWithoutBom->fromUnicode(filename);

        un = bs(un);
        fn = bs(fn);
        an = bs(an);

        un.truncate(0x40);
        fn.truncate(0x100);
        payload = payload.replace(0x008, an.size(), an);
        payload = payload.replace(0x048, un.size(), un);
        payload = payload.replace(0x088, fn.size(), fn);

        qDebug() << payload.toHex();

        QByteArray data = makePacket(CPNP_MSG_STARTTCP, payload);
        QByteArray response = exchangePackets(data);
        mTcpPort = qFromBigEndian<quint16>(response.mid(20, 20));
        mJobSeq = response.mid(10,4);
        //qDebug() << "Got TCP Port: " << mTcpPort;
        if(mTcpPort == 0) {
            qDebug() << "Printer not ready to accept data!";
            mJobState = JOB_STATE_IDLE;
            return;
        }
        connectTcp(true);
        mJobState = JOB_STATE_TCP_CONNECTING;
        break;
    }
    case JOB_STATE_TCP_CONNECTING:
        qDebug() << "Waiting for TCP connection...";
        break;
    case JOB_STATE_TCP_STARTED:
    {
        if(mData.mid(16) == mLastData.mid(16)) {
            //qDebug() << "Status already seen, polling again";
            QTimer::singleShot(500, this, SLOT(pollTcpDelayed()));
            return;
        }

        mLastData = mData;
        int state = 0;
        state |= mData.mid(16 + 18, 1).at(0);
        qDebug() << "Got state: " << state;
        qDebug() << mData.mid(16);
        switch(state) {
        case 0x00:
        {
            qDebug() << "Wait";
            quint32 err_code = qFromBigEndian<quint32>(mData.mid(16 + 12, 4));
            if(err_code > 0) {
                emit printerError("media", "The printer is either out of paper or out of ribbon and needs your attention!");
            }
            QTimer::singleShot(500, this, SLOT(pollTcpDelayed()));
            break;
        }
        case 0x01:
            qDebug() << "Send flags";
            mJobState = JOB_STATE_FLAGS_SENT;
            sendFlags();
            break;
        case 0x02:
        {
            qDebug() << "File data request";
            quint32 offset = qFromLittleEndian<quint32>(mData.mid(16+24,4).constData());
            quint32 length = qFromLittleEndian<quint32>(mData.mid(16+28,4).constData());
            //qDebug() << "Will send " << length << " bytes starting from " << offset;
            if(!mCurrentFile->seek(offset)) {
                qDebug() << "Error seeking in file.";
            }
            mFileData = makeFileHeader(offset, length);
            mFileData.append(mCurrentFile->read(length));
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
        //qDebug() << "Poll";
        mJobState = JOB_STATE_TCP_STARTED;
        pollTcpDelayed();
        break;
    case JOB_STATE_CHUNK_SENT:
        //qDebug() << "Chunk sent";
        if(mFileData.size() > 0) {
            sendChunk();
        } else {
            mJobState = JOB_STATE_TCP_STARTED;
            pollTcpDelayed();
        }
        break;
    case JOB_STATE_DONE:
        mCurrentFile->close();
        mPrintJobs.first().setCopiesPrinted(mPrintJobs.first().getCopiesPrinted() + 1);
        qDebug() << "So far, I printed " << mPrintJobs.first().getCopiesPrinted() << " copies.";
        if(mPrintJobs.first().getCopiesPrinted() >= mPrintJobs.first().getCopies()) {
            qDebug() << "Removing job as all copies have been printed.";
            mPrintJobs.removeFirst();
        }
        mTcpSocket->close();
        mJobState = JOB_STATE_IDLE;
        break;
    case JOB_STATE_FAILED:
        mCurrentFile->close();
        mJobState = JOB_STATE_IDLE;
        break;
    }
}

void printerSelphy::sendJobDone() {
    QByteArray payload(0x40, '\x0');
    const quint32 bs = qToLittleEndian<quint32>(payload.size());
    payload.replace(0x04, 4, (const char *)&bs, 4);
    payload.replace(2, 1, "\x3", 1);
    QByteArray data = makePacket(CPNP_MSG_DATA, payload);
    mJobState = JOB_STATE_DONE;
    data = data.replace(10, 4, mJobSeq);
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
    data = data.replace(10, 4, mJobSeq);
    mTcpSocket->write(data);
}

void printerSelphy::sendFlags() {
    QByteArray payload(0x40, '\x0');
    const quint32 bs = qToLittleEndian<quint32>(payload.size());
    const quint32 one = qToLittleEndian<quint32>(1);
    payload.replace(0x04, 4, (const char *)&bs, 4);
    payload.replace(0x0c,4, (const char*)&one, 4);
    // No border: 2
    // Allow border: 3
    const quint32 crop = qToLittleEndian<quint32>(2);
    payload.replace(0x12, 4, (const char*)&crop, 4);

    QByteArray data = makePacket(CPNP_MSG_DATA, payload);
    data = data.replace(10, 4, mJobSeq);
    mTcpSocket->write(data);
}

void printerSelphy::pollTcpDelayed()
{
    //qDebug() << "pollTcpDelayed";
    QByteArray data = makePacket(CPNP_MSG_STATUS, QByteArray());
    data = data.replace(10, 4, mJobSeq);
    mTcpSocket->write(data);
}

void printerSelphy::tcpConnected()
{
    qDebug() << "tcpConnected";
    mJobState = JOB_STATE_TCP_STARTED;
    QByteArray data = makePacket(CPNP_MSG_STATUS, QByteArray());
    data = data.replace(10, 4, mJobSeq);
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
