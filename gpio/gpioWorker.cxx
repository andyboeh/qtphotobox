#include "gpioWorker.h"
#include "settings.h"
#include "statemachine.h"
#include <QTimer>
#include <pigpiod_if2.h>

static gpioWorker* mWorker = nullptr;

static void cbFunc(int pi, unsigned int user_gpio, unsigned int level, uint32_t tick) {
    qDebug() << "cbFunc";
    if(mWorker) {
        QMetaObject::invokeMethod(mWorker, "callbackFunc", Qt::QueuedConnection, Q_ARG(unsigned int, user_gpio), Q_ARG(unsigned int, level));
    }
}

gpioWorker::gpioWorker()
{
    mTimer = nullptr;
    mGpioMapping.clear();
    mPWMRampSize = 1;
    mWorker = this;
}

gpioWorker::~gpioWorker()
{
    mTimer->deleteLater();
    mWorker = nullptr;
}

bool gpioWorker::setupGpio() {
    int ret;
    pbSettings &pbs = pbSettings::getInstance();

    mGpioMapping.insert("exit_pin", pbs.getInt("gpio", "exit_pin"));
    mGpioMapping.insert("trigger_pin", pbs.getInt("gpio", "trigger_pin"));
    mGpioMapping.insert("idle_lamp_pin", pbs.getInt("gpio", "idle_lamp_pin"));
    mGpioMapping.insert("idle_lamp_pwm_value", pbs.getInt("gpio", "idle_lamp_pwm_value"));
    mGpioMapping.insert("af_lamp_pin", pbs.getInt("gpio", "af_lamp_pin"));
    mGpioMapping.insert("af_lamp_pwm_value", pbs.getInt("gpio", "af_lamp_pwm_value"));

    mPi = pigpio_start(NULL, NULL);
    if(mPi < 0)
        goto out;

    // Input Pins
    ret = set_mode(mPi, mGpioMapping.value("trigger_pin"), PI_INPUT);
    if(ret != 0)
        goto out;

    ret = set_pull_up_down(mPi, mGpioMapping.value("trigger_pin"), PI_PUD_UP);
    if(ret != 0)
        goto out;

    ret = set_glitch_filter(mPi, mGpioMapping.value("trigger_pin"), 1000);
    if(ret != 0)
        goto out;

    ret = callback(mPi, mGpioMapping.value("trigger_pin"), FALLING_EDGE, cbFunc);
    if(ret < 0)
        goto out;

    ret = set_mode(mPi, mGpioMapping.value("exit_pin"), PI_INPUT);
    if(ret != 0)
        goto out;

    ret = set_pull_up_down(mPi, mGpioMapping.value("exit_pin"), PI_PUD_UP);
    if(ret != 0)
        goto out;

    ret = set_glitch_filter(mPi, mGpioMapping.value("exit_pin"), 1000);
    if(ret != 0)
        goto out;

    ret = callback(mPi, mGpioMapping.value("exit_pin"), FALLING_EDGE, cbFunc);
    if(ret < 0)
        goto out;

    ret = set_mode(mPi, mGpioMapping.value("idle_lamp_pin"), PI_OUTPUT);
    if(ret != 0)
        goto out;

    ret = set_mode(mPi, mGpioMapping.value("af_lamp_pin"), PI_OUTPUT);
    if(ret != 0)
        goto out;

    set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), 0);
    set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), 0);

    return true;

out:
    qDebug() << "Error initializing GPIO";
    return false;
}

void gpioWorker::rampTimeout()
{
    int afValue = get_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"));
    int idleValue = get_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"));

    if((afValue == mAfTarget) && (idleValue == mIdleTarget))
        mTimer->stop();

    if(afValue > mAfTarget) {
        afValue = afValue - mPWMRampSize;
        if(afValue < mAfTarget)
            afValue = mAfTarget;
    } else {
        afValue = afValue + mPWMRampSize;
        if(afValue > mAfTarget)
            afValue = mAfTarget;
    }

    if(idleValue > mIdleTarget) {
        idleValue = idleValue - mPWMRampSize;
        if(idleValue < mIdleTarget)
            idleValue = mIdleTarget;
    } else {
        idleValue = idleValue + mPWMRampSize;
        if(idleValue > mIdleTarget)
            idleValue = mIdleTarget;
    }

    set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), afValue);
    set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), idleValue);
}

void gpioWorker::start()
{
    bool running = true;

    qDebug() << "gpioThread start.";

    mTimer = new QTimer();
    mTimer->setInterval(50);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(rampTimeout()));

    if(!setupGpio())
        emit gpioError(tr("Error initializing GPIO."));

    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            continue;

        mMutex.lock();
        QString command = mCommandList.takeFirst();
        mMutex.unlock();

        if(command == "setState") {
            if(mState == "idle") {
                mAfTarget = 0;
                mIdleTarget = mGpioMapping.value("idle_lamp_pwm_value");
                mTimer->start();
            } else if(mState == "greeter" || mState == "init") {
                mAfTarget = mGpioMapping.value("af_lamp_pwm_value");
                mIdleTarget = 0;
                mTimer->start();
            } else if(mState == "archive") {
                mAfTarget = 0;
                mIdleTarget = 0;
                mTimer->start();
            } else if(mState == "review") {
                mAfTarget = 0;
                mIdleTarget = 0;
                mTimer->start();
            }
        } else if(command == "initGpio") {
            if(!setupGpio())
                emit gpioError(tr("Error initializing GPIO."));
        } else if(command == "triggerActivated") {
            StateMachine &sm = StateMachine::getInstance();
            if(mState == "idle") {
                sm.triggerNextState();
            }
        } else if(command == "exitActivated") {
            StateMachine &sm = StateMachine::getInstance();
            sm.triggerState("teardown");
        } else if(command == "stopThread") {
            set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), 0);
            set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), 0);
            pigpio_stop(mPi);
            mTimer->stop();
            running = false;
        }
    }

    emit finished();
}

void gpioWorker::callbackFunc(unsigned int gpio, unsigned int level) {
    if(gpio == (unsigned int)mGpioMapping.value("trigger_pin")) {
        qDebug() << "Trigger activated!";
        mMutex.lock();
        mCommandList.append("triggerActivated");
        mMutex.unlock();
    } else if(gpio == (unsigned int)mGpioMapping.value("exit_pin")) {
        qDebug() << "Exit activated!";
        mMutex.lock();
        mCommandList.append("exitActivated");
        mMutex.unlock();
    }
}

void gpioWorker::initGpio()
{
    mMutex.lock();
    mCommandList.append("initGpio");
    mMutex.unlock();
}

void gpioWorker::stop()
{
    mMutex.lock();
    mCommandList.append("stopThread");
    mMutex.unlock();
}

void gpioWorker::setState(QString state)
{
    mMutex.lock();
    mState = state;
    mCommandList.append("setState");
    mMutex.unlock();
}
