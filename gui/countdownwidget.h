#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

#include <QFrame>
#include <QPixmap>

namespace Ui {
class countdownWidget;
}

class QTimer;
class pbCamera;

class countdownWidget : public QFrame
{
    Q_OBJECT

public:
    explicit countdownWidget(pbCamera *cameraObject, QFrame *parent = nullptr);
    ~countdownWidget();

public slots:
    void setPreviewImage(QPixmap image);

private slots:
    void handleTimeout(void);

    void changeEvent(QEvent *event);
signals:
    void startPreview();
    void stopPreview();
private:   
    Ui::countdownWidget *ui;
    QTimer *mTimer;
    int mCountdown;
    QImage mImage;
    pbCamera *mCameraObject;
};

#endif // COUNTDOWNWIDGET_H
