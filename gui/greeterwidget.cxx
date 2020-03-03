#include "greeterwidget.h"
#include "ui_greeterwidget.h"
#include "settings.h"
#include "statemachine.h"
#include <QTimer>

greeterWidget::greeterWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::greeterWidget)
{
    ui->setupUi(this);
    pbSettings &pbs = pbSettings::getInstance();
    int numx = pbs.getInt("picture", "num_x");
    int numy = pbs.getInt("picture", "num_y");
    int numskip;
    QString skip = pbs.get("picture", "skip");
    if(skip.isEmpty()) {
        numskip = 0;
    } else {
        numskip = skip.split(",").length();
    }

    int timeout = pbs.getInt("photobooth", "greeter_time");

    mTimer = new QTimer();
    mTimer->setInterval(timeout * 1000);
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(on_btnStartCountdown_clicked()));

    int numpics = numx * numy - numskip;
    QString text = QString(tr("for %1 pictures")).arg(numpics);
    ui->lblNumPictures->setText(text);
    mTimer->start();
}

greeterWidget::~greeterWidget()
{
    mTimer->stop();
    delete mTimer;
    delete ui;
}

void greeterWidget::on_btnStartCountdown_clicked()
{
    mTimer->stop();
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}
