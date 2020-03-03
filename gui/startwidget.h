#ifndef STARTWIDGET_H
#define STARTWIDGET_H

#include <QWidget>

namespace Ui {
class startWidget;
}

class startWidget : public QWidget
{
    Q_OBJECT

public:
    explicit startWidget(QWidget *parent = nullptr);
    ~startWidget();

private slots:
    void on_btnStart_clicked();

    void on_btnSetDateTime_clicked();

    void on_btnSettings_clicked();

    void on_btnQuit_clicked();

private:
    Ui::startWidget *ui;
};

#endif // STARTWIDGET_H
