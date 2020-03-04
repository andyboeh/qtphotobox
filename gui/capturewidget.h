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
    explicit captureWidget(pbCamera *cameraObject);
    ~captureWidget();
public slots:
    void imageCaptured(QPixmap image);
signals:
    void captureImage(void);
private:
    Ui::captureWidget *ui;
    pbCamera *mCameraObject;
};

#endif // CAPTUREWIDGET_H
