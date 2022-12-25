#include "assemblewidget.h"
#include "ui_assemblewidget.h"
#include "waitingspinnerwidget.h"
#include "settings.h"
#include "statemachine.h"
#include <QVBoxLayout>
#include <QDebug>

assembleWidget::assembleWidget(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::assembleWidget)
{
    ui->setupUi(this);
    mSpinner = new WaitingSpinnerWidget();
    mSpinner->setInnerRadius(20);
    mSpinner->setLineWidth(4);
    mSpinner->setLineLength(15);
    mSpinner->start();
    layout()->addWidget(mSpinner);
}

assembleWidget::~assembleWidget()
{
    delete ui;
}

void assembleWidget::paintEvent(QPaintEvent *event) {
    QColor fg = ui->label->palette().color(QPalette::WindowText);
    if(mSpinner->color() != fg) {
        mSpinner->setColor(fg);
    }
}

void assembleWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
