#include "greeterwidget.h"
#include "ui_greeterwidget.h"
#include "settings.h"
#include "statemachine.h"
#include <QTimer>

greeterWidget::greeterWidget(QFrame *parent) :
    QFrame(parent),
    ui(new Ui::greeterWidget)
{
    ui->setupUi(this);
    pbSettings &pbs = pbSettings::getInstance();

    int timeout = pbs.getInt("qtphotobox", "greeter_time");

    mTimer = new QTimer();
    mTimer->setInterval(timeout * 1000);
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(on_btnStartCountdown_clicked()));

    int numpics = pbs.getInt("picture", "num_pictures");
    QString text = QString(tr("for %1 pictures")).arg(numpics);
    ui->lblNumPictures->setText(text);
    mTimer->start();
}

greeterWidget::~greeterWidget()
{
    mTimer->stop();
    mTimer->deleteLater();
    delete ui;
}

void greeterWidget::on_btnStartCountdown_clicked()
{
    mTimer->stop();
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void greeterWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
