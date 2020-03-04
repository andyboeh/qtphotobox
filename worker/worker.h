#ifndef _WORKER_H
#define _WORKER_H

#include <QObject>
#include <QDebug>
#include <QThread>
#include <QAbstractEventDispatcher>

class Worker : public QObject
{
    Q_OBJECT

public:
    explicit Worker(QObject *parent = 0)
        : QObject(parent)
    {
    }

signals:
    void started();
    void finished();

public slots:

    void cancel() {
        mCommandList.append("stopThread");
    }

protected:

    void waitForCommand() {
        auto const dispatcher = QThread::currentThread()->eventDispatcher();
        if(!dispatcher) {
            qCritical() << "thread with no dispatcher.";
            return;
        }
        while(mCommandList.isEmpty()) {
            dispatcher->processEvents(QEventLoop::WaitForMoreEvents);
        }
    }

    bool checkForNewCommand() {
        auto const dispatcher = QThread::currentThread()->eventDispatcher();
        if(!dispatcher) {
            qCritical() << "thread with no dispatcher.";
            return false;
        }
        dispatcher->processEvents(QEventLoop::AllEvents);
        return !mCommandList.isEmpty();
    }

    QStringList mCommandList;

};

#endif //_WORKER_H
