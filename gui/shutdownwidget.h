#ifndef SHUTDOWNWIDGET_H
#define SHUTDOWNWIDGET_H

#include <QFrame>

namespace Ui {
class shutdownWidget;
}

class shutdownWidget : public QFrame
{
    Q_OBJECT

public:
    explicit shutdownWidget(QWidget *parent = nullptr);
    ~shutdownWidget();

private slots:
    void on_btnReallyCancel_clicked();

    void on_btnReallyShutdown_clicked();

    void on_btnReallyQuit_clicked();

private:
    Ui::shutdownWidget *ui;
};

#endif // SHUTDOWNWIDGET_H
