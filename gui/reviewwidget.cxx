#include "reviewwidget.h"
#include "ui_reviewwidget.h"
#include "settings.h"
#include <QTimer>
#include "statemachine.h"
#include <QDebug>

reviewWidget::reviewWidget(QPixmap image, QFrame *parent) :
    QFrame(parent),
    ui(new Ui::reviewWidget)
{
    ui->setupUi(this);

    mImage = image;

    mTimer = new QTimer();

    pbSettings &pbs = pbSettings::getInstance();
    int timeout = pbs.getInt("qtphotobox", "display_time");
    mTimer->setInterval(timeout * 1000);
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    mTimer->start();
    mWidth = 0;
    mHeight = 0;
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

void reviewWidget::paintEvent(QPaintEvent *event)
{
    if(ui->lblReviewImage->size().width() != mWidth || ui->lblReviewImage->size().height() != mHeight) {
        mScaledImage = mImage.scaled(ui->lblReviewImage->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        mWidth = ui->lblReviewImage->size().width();
        mHeight = ui->lblReviewImage->size().height();
        ui->lblReviewImage->setPixmap(mScaledImage);
    }



}
