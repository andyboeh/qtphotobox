#ifndef IDLEWIDGET_H
#define IDLEWIDGET_H

#include <QWidget>

namespace Ui {
class idleWidget;
}

class idleWidget : public QWidget
{
    Q_OBJECT

public:
    explicit idleWidget(QWidget *parent = nullptr);
    ~idleWidget();

private slots:
    void on_btnTrigger_clicked();

private:
    Ui::idleWidget *ui;
};

#endif // IDLEWIDGET_H
