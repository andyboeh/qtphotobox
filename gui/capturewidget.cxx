#include "capturewidget.h"
#include "ui_capturewidget.h"
#include "pbcamera.h"
#include <QTimer>
#include "settings.h"
#include "statemachine.h"

captureWidget::captureWidget(pbCamera *cameraObject) :
    ui(new Ui::captureWidget)
{
    ui->setupUi(this);
    mCameraObject = cameraObject;
    connect(this, SIGNAL(captureImage()), mCameraObject, SLOT(captureImage()));
    connect(mCameraObject, SIGNAL(imageCaptured(QPixmap)), this, SLOT(imageCaptured(QPixmap)));
    mTimer = new QTimer();

    pbSettings &pbs = pbSettings::getInstance();
    int timeout = pbs.getInt("photobooth", "display_time");
    mTimer->setInterval(timeout * 1000);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(timeout()));

    emit captureImage();
}

captureWidget::~captureWidget()
{
    delete ui;
    delete mTimer;
}

void captureWidget::imageCaptured(QPixmap image)
{
    ui->lblCapture->setPixmap(image);
    mTimer->start();
}

void captureWidget::timeout() {
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerNextState();
}
