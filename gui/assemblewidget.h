#ifndef ASSEMBLEWIDGET_H
#define ASSEMBLEWIDGET_H

#include <QFrame>
class WaitingSpinnerWidget;

namespace Ui {
class assembleWidget;
}

class assembleWidget : public QFrame
{
    Q_OBJECT

public:
    explicit assembleWidget(QWidget *parent = nullptr);
    ~assembleWidget();
private slots:
    void changeEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
private:
    Ui::assembleWidget *ui;
    WaitingSpinnerWidget *mSpinner;
};

#endif // ASSEMBLEWIDGET_H
