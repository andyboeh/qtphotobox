#ifndef GREETERWIDGET_H
#define GREETERWIDGET_H

#include <QFrame>

namespace Ui {
class greeterWidget;
}

class QTimer;

class greeterWidget : public QFrame
{
    Q_OBJECT

public:
    explicit greeterWidget(QFrame *parent = nullptr);
    ~greeterWidget();

private slots:
    void on_btnStartCountdown_clicked();

private:
    Ui::greeterWidget *ui;
    QTimer *mTimer;
};

#endif // GREETERWIDGET_H
