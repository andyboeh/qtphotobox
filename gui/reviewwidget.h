#ifndef REVIEWWIDGET_H
#define REVIEWWIDGET_H

#include <QFrame>

namespace Ui {
class reviewWidget;
}

class QTimer;

class reviewWidget : public QFrame
{
    Q_OBJECT

public:
    explicit reviewWidget(QWidget *parent = nullptr);
    ~reviewWidget();
public slots:
    void timeout(void);
private:
    Ui::reviewWidget *ui;
    QTimer *mTimer;
};

#endif // REVIEWWIDGET_H
