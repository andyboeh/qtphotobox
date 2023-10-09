#include "idlewidget.h"
#include "ui_idlewidget.h"
#include "settings.h"
#include "statemachine.h"

idleWidget::idleWidget(QFrame *parent) :
    QFrame(parent),
    ui(new Ui::idleWidget)
{
    ui->setupUi(this);
    pbSettings &pbs = pbSettings::getInstance();
    if(pbs.getBool("archive", "enable")) {
        ui->btnArchive->setVisible(true);
    } else {
        ui->btnArchive->setVisible(false);
    }
    if(pbs.getBool("gui", "shutdown")) {
        ui->btnShutdown->setVisible(true);
    } else {
        ui->btnShutdown->setVisible(false);
    }
}

idleWidget::~idleWidget()
{
    delete ui;
}

void idleWidget::on_btnTrigger_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void idleWidget::on_btnArchive_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("archive");
}

void idleWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}

void idleWidget::on_btnShutdown_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("askteardown");
}

