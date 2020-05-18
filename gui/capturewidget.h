#ifndef CAPTUREWIDGET_H
#define CAPTUREWIDGET_H

#include <QFrame>
#include <QPixmap>

namespace Ui {
class captureWidget;
}

class pbCamera;

class captureWidget : public QFrame
{
    Q_OBJECT

public:
    explicit captureWidget(pbCamera *cameraObject, QFrame *parent = nullptr);
    ~captureWidget();
public slots:
    void imageCaptured(QPixmap image);
signals:
    void captureImage(void);
private slots:
    void changeEvent(QEvent *event);
private:
    Ui::captureWidget *ui;
    pbCamera *mCameraObject;
};

#endif // CAPTUREWIDGET_H
