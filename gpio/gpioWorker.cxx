#include "gpioWorker.h"
#include "settings.h"
#include <QTimer>
#include <pigpiod_if2.h>

gpioWorker::gpioWorker()
{
    mTimer = nullptr;
    mGpioMapping.clear();
    mPWMRampSize = 5;
}

gpioWorker::~gpioWorker()
{
    delete mTimer;
    mTimer = nullptr;
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

    ret = set_mode(mPi, mGpioMapping.value("exit_pin"), PI_INPUT);
    if(ret != 0)
        goto out;

    ret = set_mode(mPi, mGpioMapping.value("idle_lamp_pin"), PI_OUTPUT);
    if(ret != 0)
        goto out;

    ret = set_mode(mPi, mGpioMapping.value("af_lamp_pin"), PI_OUTPUT);
    if(ret != 0)
        goto out;

    //ret = callback(mPi, mGpioMapping.value("trigger_pin"), FALLING_EDGE, &cbFunc);
    //ret = callback(mPi, mGpioMapping.value("exit_pin"), FALLING_EDGE, &cbFunc);

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

        QString command = mCommandList.takeFirst();

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

void gpioWorker::initGpio()
{
    mCommandList.append("initGpio");
}

void gpioWorker::stop()
{
    mCommandList.append("stopThread");
}

void gpioWorker::setState(QString state)
{
    mState = state;
    mCommandList.append("setState");
}
