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
private slots:
    void on_btnQuit_clicked();

    void changeEvent(QEvent *event);
private:
    Ui::waitRemovableWidget *ui;
};

#endif // WAITREMOVABLEWIDGET_H
