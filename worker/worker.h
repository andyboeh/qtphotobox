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
    void pause()
    {
        auto const dispatcher = QThread::currentThread()->eventDispatcher();
        if (!dispatcher) {
            qCritical() << "thread with no dispatcher";
            return;
        }

        if (state != RUNNING)
            return;

        state = PAUSED;
        qDebug() << "paused";
        do {
            dispatcher->processEvents(QEventLoop::WaitForMoreEvents);
        } while (state == PAUSED);
    }

    void resume()
    {
        if (state == PAUSED) {
            state = RUNNING;
            qDebug() << "resumed";
        }
    }

    void cancel() {
        if (state != IDLE) {
            state = IDLE;
            qDebug() << "cancelled";
        }
    }

protected:

    enum State { IDLE, RUNNING, PAUSED };
    State state = IDLE;

    bool isCancelled() {
        auto const dispatcher = QThread::currentThread()->eventDispatcher();
        if (!dispatcher) {
            qCritical() << "thread with no dispatcher";
            return false;
        }
        dispatcher->processEvents(QEventLoop::AllEvents);
        return state == IDLE;
    }

};

#endif //_WORKER_H
