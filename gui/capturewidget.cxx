#include "capturewidget.h"
#include "ui_capturewidget.h"
#include "pbcamera.h"
#include "statemachine.h"

captureWidget::captureWidget(pbCamera *cameraObject, QFrame *parent) :
    QFrame(parent),
    ui(new Ui::captureWidget)
{
    ui->setupUi(this);
    mCameraObject = cameraObject;
    connect(this, SIGNAL(captureImage()), mCameraObject, SLOT(captureImage()));
    connect(mCameraObject, SIGNAL(imageCaptured(QPixmap)), this, SLOT(imageCaptured(QPixmap)));

    emit captureImage();
}

captureWidget::~captureWidget()
{
    delete ui;
}

void captureWidget::imageCaptured(QPixmap image)
{
    ui->lblCapture->setPixmap(image);
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}

void captureWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
