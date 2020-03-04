#include "reviewwidget.h"
#include "ui_reviewwidget.h"
#include "settings.h"
#include <QTimer>
#include "statemachine.h"

reviewWidget::reviewWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::reviewWidget)
{
    ui->setupUi(this);

    mTimer = new QTimer();

    pbSettings &pbs = pbSettings::getInstance();
    int timeout = pbs.getInt("photobooth", "display_time");
    mTimer->setInterval(timeout * 1000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
}

reviewWidget::~reviewWidget()
{
    delete ui;
    delete mTimer;
}

void reviewWidget::timeout() {
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}
