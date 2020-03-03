#ifndef COUNTDOWNWIDGET_H
#define COUNTDOWNWIDGET_H

#include <QWidget>
#include <QPixmap>

namespace Ui {
class countdownWidget;
}

class QTimer;
class pbCamera;

class countdownWidget : public QWidget
{
    Q_OBJECT

public:
    explicit countdownWidget(pbCamera *cameraObject);
    ~countdownWidget();

public slots:
    void setPreviewImage(QPixmap image);

private slots:
    void handleTimeout(void);

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
