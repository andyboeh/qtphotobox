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
