#ifndef WAITREMOVABLEWIDGET_H
#define WAITREMOVABLEWIDGET_H

#include <QFrame>

namespace Ui {
class waitRemovableWidget;
}

class waitRemovableWidget : public QFrame
{
    Q_OBJECT

public:
    explicit waitRemovableWidget(QWidget *parent = nullptr);
    ~waitRemovableWidget();
public slots:
    void removableDeviceDetected(QString path);
private slots:
    void on_btnQuit_clicked();

private:
    Ui::waitRemovableWidget *ui;
};

#endif // WAITREMOVABLEWIDGET_H
