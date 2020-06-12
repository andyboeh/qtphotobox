#include "screensaver.h"
#include "statemachine.h"
#include <QDebug>

screenSaver::screenSaver()
{
    mTimeout = 0;
    mIsEnabled = false;
    mTargetState = "";
    mTimer.setSingleShot(true);
    connect(&mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

screenSaver::~screenSaver()
{
    mTimer.stop();
}

void screenSaver::enableScreenSaver()
{
    mIsEnabled = true;
}

void screenSaver::disableScreenSaver()
{
    mIsEnabled = false;
    mTimer.stop();
}

void screenSaver::setTimeout(int timeout)
{
    mTimeout = timeout * 1000;
    if(mTimer.isActive()) {
        mTimer.stop();
        mTimer.setInterval(mTimeout);
        mTimer.start();
    }
}

int screenSaver::getTimeout()
{
    return mTimeout / 1000;
}

void screenSaver::setTargetState(QString state)
{
    mTargetState = state;
}

QString screenSaver::getTargetState()
{
    return mTargetState;
}

void screenSaver::timeout()
{
    qDebug() << "Timeout, activating screensaver";
    StateMachine &stm = StateMachine::getInstance();
    if(stm.getCurrentState() == mTargetState && mIsEnabled) {
        stm.triggerState("screensaver");
    }
}

void screenSaver::changeState(QString name)
{
    if(!mIsEnabled)
        return;

    if(name == mTargetState) {
        mTimer.stop();
        mTimer.setInterval(mTimeout);
        mTimer.start();
    } else {
        mTimer.stop();
    }
}
