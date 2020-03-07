#ifndef INITWIDGET_H
#define INITWIDGET_H

#include <QFrame>

namespace Ui {
class initWidget;
}

class QTimer;

class initWidget : public QFrame
{
    Q_OBJECT

public:
    explicit initWidget(QFrame *parent = nullptr);
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
    int mIncrement;
};

#endif // INITWIDGET_H
