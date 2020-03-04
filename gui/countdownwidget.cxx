#include "countdownwidget.h"
#include "ui_countdownwidget.h"
#include "settings.h"
#include "statemachine.h"
#include "pbcamera.h"
#include <QTimer>

static const int fps = 10;

countdownWidget::countdownWidget(pbCamera *cameraObject) :
    ui(new Ui::countdownWidget)
{
    ui->setupUi(this);
    pbSettings &pbs = pbSettings::getInstance();

    mCameraObject = cameraObject;
    connect(this, SIGNAL(startPreview()), mCameraObject, SLOT(startPreview()));
    connect(this, SIGNAL(stopPreview()), mCameraObject, SLOT(stopPreview()));
    connect(mCameraObject, SIGNAL(previewImageCaptured(QPixmap)), this, SLOT(setPreviewImage(QPixmap)));

    mCountdown = pbs.getInt("photobooth", "countdown_time") * fps;

    ui->progressBar->setMaximum(mCountdown / fps);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(mCountdown / fps);

    mTimer = new QTimer();
    mTimer->setInterval(1000 / fps);
    mTimer->setSingleShot(false);
    connect(mTimer, SIGNAL(timeout()), this, SLOT(handleTimeout()));
    mTimer->start();
    emit startPreview();
}

countdownWidget::~countdownWidget()
{
    emit stopPreview();
    mTimer->stop();
    delete ui;
    delete mTimer;
}

void countdownWidget::setPreviewImage(QPixmap image)
{
    ui->lblPreview->setPixmap(image);
}

void countdownWidget::handleTimeout()
{
    mCountdown--;
    ui->progressBar->setValue(mCountdown/fps + 1);

    if(mCountdown == 0) {
        emit stopPreview();
        mTimer->stop();
        StateMachine &sm = StateMachine::getInstance();
        sm.triggerNextState();
    }
}