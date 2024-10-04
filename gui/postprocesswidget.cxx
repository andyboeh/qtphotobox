#include "postprocesswidget.h"
#include "ui_postprocesswidget.h"
#include "statemachine.h"
#include "settings.h"
#include "passwordwidget.h"
#include <QDebug>

postprocessWidget::postprocessWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::postprocessWidget)
{
    ui->setupUi(this);
    if(parent) {
        QRect parentRect = parent->rect();
        setGeometry(parentRect);
    }
    pbSettings &pbs = pbSettings::getInstance();
    if(pbs.getBool("printer", "enable") && pbs.getBool("printer", "allowprinting")) {
        ui->btnPrint->setVisible(true);
        int maxcopies = pbs.getInt("printer", "max_copies");
        if(maxcopies > 1) {
            ui->sliderNumCopies->setVisible(true);
            ui->lblNumCopies->setVisible(true);
        } else {
            ui->sliderNumCopies->setVisible(false);
            ui->lblNumCopies->setVisible(false);
        }
        ui->sliderNumCopies->setMinimum(1);
        ui->sliderNumCopies->setMaximum(maxcopies);
        ui->sliderNumCopies->setValue(1);
        ui->lblNumCopies->setText(tr("Print %1 copies.").arg(ui->sliderNumCopies->value()));
    } else {
        ui->btnPrint->setVisible(false);
        ui->sliderNumCopies->setVisible(false);
        ui->lblNumCopies->setVisible(false);
    }
}

postprocessWidget::~postprocessWidget()
{
    delete ui;
}

void postprocessWidget::on_btnPrint_clicked()
{
    pbSettings &pbs = pbSettings::getInstance();
    if(pbs.getBool("printer", "enforcepassword")) {
        mPasswordWidget = new passwordWidget(this);
        connect(mPasswordWidget, SIGNAL(printPicture()), this, SLOT(printFromPasswordDialog()));
        connect(mPasswordWidget, SIGNAL(cancelled()), this, SLOT(passwordDialogClosed()));
        mPasswordWidget->show();
    } else {
        int numcopies = ui->sliderNumCopies->value();
        ui->btnPrint->setEnabled(false);
        emit startPrintJob(numcopies);
    }
}

void postprocessWidget::printFromPasswordDialog() {
    delete mPasswordWidget;
    mPasswordWidget = nullptr;

    ui->btnPrint->setEnabled(false);
    int numcopies = ui->sliderNumCopies->value();
    emit startPrintJob(numcopies);
}

void postprocessWidget::passwordDialogClosed() {
    delete mPasswordWidget;
    mPasswordWidget = nullptr;
}

void postprocessWidget::on_btnStartOver_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void postprocessWidget::on_sliderNumCopies_valueChanged(int value)
{
    ui->lblNumCopies->setText(tr("Print %1 copies.").arg(value));
}

void postprocessWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
