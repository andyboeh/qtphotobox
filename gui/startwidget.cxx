#include "startwidget.h"
#include "statemachine.h"
#include "ui_startwidget.h"

startWidget::startWidget(QFrame *parent) :
    QFrame(parent),
    ui(new Ui::startWidget)
{
    ui->setupUi(this);
}

startWidget::~startWidget()
{
    delete ui;
}

void startWidget::on_btnStart_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void startWidget::on_btnSetDateTime_clicked()
{

}

void startWidget::on_btnSettings_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("settings");
}

void startWidget::on_btnQuit_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("teardown");
}
