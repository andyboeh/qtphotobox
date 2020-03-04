#ifndef CAPTUREWIDGET_H
#define CAPTUREWIDGET_H

#include <QWidget>
#include <QPixmap>

namespace Ui {
class captureWidget;
}

class pbCamera;
class QTimer;

class captureWidget : public QWidget
{
    Q_OBJECT

public:
    explicit captureWidget(pbCamera *cameraObject);
    ~captureWidget();
public slots:
    void imageCaptured(QPixmap image);
    void timeout(void);
signals:
    void captureImage(void);
private:
    Ui::captureWidget *ui;
    pbCamera *mCameraObject;
    QTimer *mTimer;
};

#endif // CAPTUREWIDGET_H
