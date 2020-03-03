#ifndef INITWIDGET_H
#define INITWIDGET_H

#include <QWidget>

namespace Ui {
class initWidget;
}

class QTimer;
class pbCamera;

class initWidget : public QWidget
{
    Q_OBJECT

public:
    explicit initWidget(pbCamera *cameraObject);
    ~initWidget();
signals:
    void initializeCamera(void);
public slots:
    void cameraInitialized(bool ret);
private slots:
    void timeout(void);
private:
    Ui::initWidget *ui;
    QTimer *mTimer;
    pbCamera *mCameraObject;
    int mIncrement;
};

#endif // INITWIDGET_H
