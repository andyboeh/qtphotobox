#ifndef _SCREENSAVER_H
#define _SCREENSAVER_H

#include <QObject>
#include <QTimer>

class screenSaver : public QObject
{
    Q_OBJECT
public:
    screenSaver();
    ~screenSaver();
    void enableScreenSaver();
    void disableScreenSaver();
    void setTimeout(int timeout);
    int getTimeout();
    void setTargetState(QString state);
    QString getTargetState();
private slots:
    void timeout();
public slots:
    void changeState(QString name);
private:
    bool mIsEnabled;
    QString mTargetState;
    int mTimeout;
    QTimer mTimer;
};


#endif //_SCREENSAVER_H
