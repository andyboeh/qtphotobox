#include "waitremovablewidget.h"
#include "ui_waitremovablewidget.h"
#include "statemachine.h"

waitRemovableWidget::waitRemovableWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::waitRemovableWidget)
{
    ui->setupUi(this);
}

waitRemovableWidget::~waitRemovableWidget()
{
    delete ui;
}

void waitRemovableWidget::removableDeviceDetected(QString path)
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void waitRemovableWidget::on_btnQuit_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("teardown");
}

void waitRemovableWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
