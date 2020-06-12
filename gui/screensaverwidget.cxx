#include "screensaverwidget.h"
#include "ui_screensaverwidget.h"
#include "settings.h"
#include "statemachine.h"

screensaverWidget::screensaverWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::screensaverWidget)
{
    ui->setupUi(this);
    pbSettings &settings = pbSettings::getInstance();
    ui->lblText1->setText(settings.get("screensaver", "text1"));
    ui->lblText2->setText(settings.get("screensaver", "text2"));
    ui->lblText3->setText(settings.get("screensaver", "text3"));
    ui->lblText4->setText(tr("Touch to continue..."));
}

screensaverWidget::~screensaverWidget()
{
    delete ui;
}

void screensaverWidget::mousePressEvent(QMouseEvent *event)
{
    StateMachine &stm = StateMachine::getInstance();
    stm.triggerNextState();
}
