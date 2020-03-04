#ifndef IDLEWIDGET_H
#define IDLEWIDGET_H

#include <QFrame>

namespace Ui {
class idleWidget;
}

class idleWidget : public QFrame
{
    Q_OBJECT

public:
    explicit idleWidget(QFrame *parent = nullptr);
    ~idleWidget();

private slots:
    void on_btnTrigger_clicked();

private:
    Ui::idleWidget *ui;
};

#endif // IDLEWIDGET_H
