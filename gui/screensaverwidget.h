#ifndef SCREENSAVERWIDGET_H
#define SCREENSAVERWIDGET_H

#include <QFrame>

namespace Ui {
class screensaverWidget;
}

class screensaverWidget : public QFrame
{
    Q_OBJECT

public:
    explicit screensaverWidget(QWidget *parent = nullptr);
    ~screensaverWidget();
private slots:
    void mousePressEvent(QMouseEvent *event);
private:
    Ui::screensaverWidget *ui;
};

#endif // SCREENSAVERWIDGET_H
