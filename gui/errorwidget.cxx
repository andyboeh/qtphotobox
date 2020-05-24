#include "errorwidget.h"
#include "ui_errorwidget.h"
#include <QDebug>

errorWidget::errorWidget(errorWidgetButtons buttons, QString message, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::errorWidget)
{
    ui->setupUi(this);
    if(parent) {
        QRect parentRect = parent->rect();
        setGeometry(parentRect);
    }
    switch(buttons) {
    case BTN_OK:
        ui->btnQuit->setVisible(false);
        ui->btnRetry->setVisible(false);
        break;
    case BTN_QUIT:
        ui->btnOk->setVisible(false);
        ui->btnRetry->setVisible(false);
        break;
    case BTN_RETRY:
        ui->btnOk->setVisible(false);
        ui->btnQuit->setVisible(false);
        break;
    case BTN_OK_QUIT:
        ui->btnRetry->setVisible(false);
        break;
    case BTN_OK_RETRY:
        ui->btnQuit->setVisible(false);
        break;
    case BTN_RETRY_QUIT:
        ui->btnOk->setVisible(false);
        break;
    case BTN_OK_RETRY_QUIT:
        break;
    }
    mErrorMessage = message;
    ui->lblErrorMessage->setText(mErrorMessage);
}

errorWidget::~errorWidget()
{
    delete ui;
}

void errorWidget::on_btnRetry_clicked()
{
    emit errorRetry();
}

void errorWidget::on_btnQuit_clicked()
{
    emit errorQuit();
}

void errorWidget::on_btnOk_clicked()
{
    emit errorOk();
}

void errorWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        ui->lblErrorMessage->setText(mErrorMessage);
    }

    QFrame::changeEvent(event);
}
