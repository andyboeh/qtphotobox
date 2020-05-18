#include "countdownwidget.h"
#include "ui_countdownwidget.h"
#include "settings.h"
#include "statemachine.h"
#include "pbcamera.h"
#include <QTimer>

static const int fps = 10;

countdownWidget::countdownWidget(pbCamera *cameraObject, QFrame *parent) :
    QFrame(parent),
    ui(new Ui::countdownWidget)
{
    ui->setupUi(this);
    pbSettings &pbs = pbSettings::getInstance();

    mCameraObject = cameraObject;
    connect(this, SIGNAL(startPreview()), mCameraObject, SLOT(startPreview()));
    connect(this, SIGNAL(stopPreview()), mCameraObject, SLOT(stopPreview()));
    connect(mCameraObject, SIGNAL(previewImageCaptured(QPixmap)), this, SLOT(setPreviewImage(QPixmap)));

    mCountdown = pbs.getInt("qtphotobox", "countdown_time") * fps;

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
    mTimer->deleteLater();
}

void countdownWidget::setPreviewImage(QPixmap image)
{
    QPixmap scaledImage = image.scaled(ui->lblPreview->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->lblPreview->setPixmap(scaledImage);
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

void countdownWidget::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
    }

    QFrame::changeEvent(event);
}
