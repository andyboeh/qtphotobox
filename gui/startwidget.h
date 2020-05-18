#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QFrame>

namespace Ui {
class startWidget;
}

class startWidget : public QFrame
{
    Q_OBJECT

public:
    explicit startWidget(QFrame *parent = nullptr);
    ~startWidget();

private slots:
    void on_btnStart_clicked();

    void on_btnSetDateTime_clicked();

    void on_btnSettings_clicked();

    void on_btnQuit_clicked();

    void changeEvent(QEvent *event);

private:
    Ui::startWidget *ui;
};

#endif // STARTWIDGET_H
