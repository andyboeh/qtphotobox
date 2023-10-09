#include "shutdownwidget.h"
#include "ui_shutdownwidget.h"
#include "statemachine.h"
#include <QProcess>

shutdownWidget::shutdownWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::shutdownWidget)
{
    ui->setupUi(this);

    if(parent) {
        QRect parentRect = parent->rect();
        setGeometry(parentRect);
    }
}

shutdownWidget::~shutdownWidget()
{
    delete ui;
}

void shutdownWidget::on_btnReallyCancel_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("idle");
}


void shutdownWidget::on_btnReallyShutdown_clicked()
{
    QProcess *shutdown = new QProcess;
    shutdown->start("/sbin/shutdown -h now");
}


void shutdownWidget::on_btnReallyQuit_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("teardown");
}

