#include "assemblewidget.h"
#include "ui_assemblewidget.h"
#include "waitingspinnerwidget.h"
#include "settings.h"
#include "statemachine.h"
#include <QVBoxLayout>

assembleWidget::assembleWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::assembleWidget)
{
    ui->setupUi(this);

    WaitingSpinnerWidget *spinner = new WaitingSpinnerWidget();
    spinner->start();
    layout()->addWidget(spinner);
}

assembleWidget::~assembleWidget()
{
    delete ui;
}

void assembleWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
