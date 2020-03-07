#include "gpioWorker.h"
#include "settings.h"
#include <QTimer>
#include <pigpiod_if2.h>

gpioWorker::gpioWorker()
{
    mTimer = nullptr;
    mGpioMapping.clear();
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

void gpioWorker::start()
{
    bool running = true;

    qDebug() << "gpioThread start.";

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
                set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), 0);
                set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), mGpioMapping.value("idle_lamp_pwm_value"));
            } else if(mState == "greeter" || mState == "init") {
                set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), 0);
                set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), mGpioMapping.value("af_lamp_pwm_value"));
            } else if(mState == "archive") {
                set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), 0);
                set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), 0);
            }
        } else if(command == "initGpio") {
            if(!setupGpio())
                emit gpioError(tr("Error initializing GPIO."));
        } else if(command == "stopThread") {
            set_PWM_dutycycle(mPi, mGpioMapping.value("idle_lamp_pin"), 0);
            set_PWM_dutycycle(mPi, mGpioMapping.value("af_lamp_pin"), 0);
            pigpio_stop(mPi);
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
