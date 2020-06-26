#include "picturedetailwidget.h"
#include "ui_picturedetailwidget.h"
#include "settings.h"
#include <QDebug>

picturedetailWidget::picturedetailWidget(QString filename, QWidget *parent) :
    QFrame(parent),
    ui(new Ui::picturedetailWidget),
    mFilename(filename)
{
    ui->setupUi(this);
    if(parent) {
        QRect parentRect = parent->rect();
        setGeometry(parentRect);
    }
    pbSettings &pbs = pbSettings::getInstance();
    if(pbs.getBool("printer", "enable") && pbs.getBool("archive", "allow_reprint")) {
        ui->btnPrint->setVisible(true);
        int maxcopies = pbs.getInt("printer", "max_copies");
        if(maxcopies > 1) {
            ui->sliderCopies->setVisible(true);
            ui->lblCopies->setVisible(true);
        } else {
            ui->sliderCopies->setVisible(false);
            ui->lblCopies->setVisible(false);
        }
        ui->sliderCopies->setMinimum(1);
        ui->sliderCopies->setMaximum(maxcopies);
        ui->sliderCopies->setValue(1);
        ui->lblCopies->setText(tr("Print %1 copies.").arg(ui->sliderCopies->value()));
    } else {
        ui->btnPrint->setVisible(false);
        ui->sliderCopies->setVisible(false);
        ui->lblCopies->setVisible(false);
    }
    mWidth = 0;
    mHeight = 0;
}

picturedetailWidget::~picturedetailWidget()
{
    delete ui;
}

void picturedetailWidget::on_btnPrint_clicked()
{
    int copies = ui->sliderCopies->value();
    ui->btnPrint->setEnabled(false);
    ui->sliderCopies->setEnabled(false);
    ui->lblCopies->setEnabled(false);
    emit printArchivePicture(mFilename, copies);
}

void picturedetailWidget::on_btnClose_clicked()
{
    hide();
}

void picturedetailWidget::on_sliderCopies_valueChanged(int value)
{
    ui->lblCopies->setText(tr("Print %1 copies.").arg(value));
}

void picturedetailWidget::paintEvent(QPaintEvent *event)
{
    qDebug() << "paintEvent";
    if(ui->lblPicture->size().width() != mWidth || ui->lblPicture->size().height() != mHeight) {
        qDebug() << "not resized.";
        QPixmap image(mFilename);
        if(image.isNull())
            return;
        qDebug() << "pixmap not null";
        mScaledImage = image.scaled(ui->lblPicture->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
        mWidth = ui->lblPicture->size().width();
        mHeight = ui->lblPicture->size().height();
        ui->lblPicture->setPixmap(mScaledImage);
    }

}

void picturedetailWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
