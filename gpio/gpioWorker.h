#ifndef _GPIOWORKER_H
#define _GPIOWORKER_H

#include "worker.h"
#include <QString>
#include <QMap>

class QTimer;

class gpioWorker : public Worker
{
    Q_OBJECT
public:
    gpioWorker();
    ~gpioWorker();
public slots:
    void start(void);
    void stop(void);
    void setState(QString state);
    void initGpio();
signals:
    void gpioError(QString message);
private slots:
    void rampTimeout();
private:
    int mPWMRampSize;
    int mIdleTarget;
    int mAfTarget;
    QTimer *mTimer;
    QString mState;
    bool setupGpio();
    QMap<QString, int> mGpioMapping;
    int mPi;
};

#endif //_GPIOWORKER_H
