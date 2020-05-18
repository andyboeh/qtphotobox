#ifndef ASSEMBLEWIDGET_H
#define ASSEMBLEWIDGET_H

#include <QFrame>

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
private:
    Ui::assembleWidget *ui;
};

#endif // ASSEMBLEWIDGET_H
