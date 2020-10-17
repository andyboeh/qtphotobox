#include "initwidget.h"
#include "ui_initwidget.h"
#include "statemachine.h"
#include <QTimer>
#include <QDebug>

initWidget::initWidget(QFrame *parent) :
    QFrame(parent),
    ui(new Ui::initWidget)
{
    ui->setupUi(this);

    ui->progressBar->setMaximum(10);
    ui->progressBar->setMinimum(0);
    mIncrement = 1;
    ui->progressBar->setValue(0);
    mTimer = new QTimer();
    mTimer->setInterval(100);
    mTimer->setSingleShot(false);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    mTimer->start();
}

initWidget::~initWidget()
{
    mTimer->stop();
    mTimer->deleteLater();
    delete ui;
}

void initWidget::cameraInitialized(bool ret)
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void initWidget::timeout()
{
    qDebug() << "timeout";
    int val = ui->progressBar->value();
    if(val == ui->progressBar->maximum())
    {
        mIncrement = -1;
    } else if(val == ui->progressBar->minimum()) {
        mIncrement = 1;
    }
    ui->progressBar->setValue(val + mIncrement);
}

void initWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
