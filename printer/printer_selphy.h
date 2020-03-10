#ifndef _PRINTER_SELPHY
#define _PRINTER_SELPHY

#include "printer_interface.h"
#include <QByteArray>
#include "printJob.h"
#include <QList>
#include <QAbstractSocket>

class QFile;
class QTcpSocket;
class QUdpSocket;
class QTimer;

class printerSelphy : public printerInterface {
    Q_OBJECT
    typedef enum {
        CPNP_MSG_DISCOVER = 0x101,
        CPNP_MSG_STARTTCP = 0x110,
        CPNP_MSG_ID       = 0x130,
        CPNP_MSG_STATUS   = 0x120,
        CPNP_MSG_DATA     = 0x121,
        CPNP_MSG_FLUSH    = 0x151
    } selphyCommands;

    typedef enum {
        SELPHY_OK,
        SELPHY_OFFLINE,
        SELPHY_RIBBON_NOT_INSTALLED,
        SELPHY_PAPER_NOT_INSTALLED,
        SELPHY_RIBBON_AND_PAPER_NOT_INSTALLED,
    } selphyState;

    typedef enum {
        JOB_STATE_DONE,
        JOB_STATE_IDLE,
        JOB_STATE_TCP_CONNECTING,
        JOB_STATE_TCP_STARTED,
        JOB_STATE_FAILED,
        JOB_STATE_FLAGS_SENT,
        JOB_STATE_CHUNK_SENT
    } selphyJobState;
public:
    printerSelphy();
    ~printerSelphy();
    bool canPrintFiles();
    bool printImage(QPixmap image, int numcopies);
    bool initPrinter();
    bool printFile(QString filename, int numcopies);
    QString getStatus();
private slots:
    void processTcpData();
    void processPrintJob(bool tcp = false);
    void pollTcpDelayed();
    void tcpConnected();
    void tcpError(QAbstractSocket::SocketError error);
    void sendChunk();
    void sendJobDone();
    void sendFlags();
private:
    QTcpSocket *mTcpSocket;
    QUdpSocket *mUdpSocket;
    QByteArray makePacket(qint16 command, QByteArray payload, qint32 options = 0);
    qint32 mSeqCount;
    QByteArray exchangePackets(QByteArray data);
    QByteArray mData;
    QByteArray mLastData;
    QString mIp;
    QList<printJob> mPrintJobs;
    QByteArray mJobSeq;
    QFile *mCurrentFile;
    QByteArray mFileData;
    QTimer *mTimer;
    int mPort;
    int mTcpPort;
    bool flush();
    bool connectUdp(bool reconnect);
    selphyState getSelphyState();
    selphyJobState mJobState;
    bool connectTcp(bool reconnect);

    QByteArray makeFileHeader(quint32 offset, quint32 length);
    QByteArray bs(QByteArray data);
};

#endif //_PRINTER_SELPHY
