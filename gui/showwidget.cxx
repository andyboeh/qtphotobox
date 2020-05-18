#include "showwidget.h"
#include "ui_showwidget.h"

showWidget::showWidget(QString footertext, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::showWidget)
{
    ui->setupUi(this);
    ui->lblStatus->setText(footertext);
    ui->lblPicture->setText(tr("Starting Show..."));
}

showWidget::~showWidget()
{
    delete ui;
}

void showWidget::showPicture(QPixmap image)
{
    ui->lblPicture->setPixmap(image.scaled(ui->lblPicture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
}

void showWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
