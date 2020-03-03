#ifndef GREETERWIDGET_H
#define GREETERWIDGET_H

#include <QWidget>

namespace Ui {
class greeterWidget;
}

class QTimer;

class greeterWidget : public QWidget
{
    Q_OBJECT

public:
    explicit greeterWidget(QWidget *parent = nullptr);
    ~greeterWidget();

private slots:
    void on_btnStartCountdown_clicked();

private:
    Ui::greeterWidget *ui;
    QTimer *mTimer;
};

#endif // GREETERWIDGET_H
