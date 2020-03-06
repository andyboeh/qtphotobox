#ifndef SHOWWIDGET_H
#define SHOWWIDGET_H

#include <QFrame>

namespace Ui {
class showWidget;
}

class showWidget : public QFrame
{
    Q_OBJECT

public:
    explicit showWidget(QString footertext, QWidget *parent = nullptr);
    ~showWidget();
public slots:
    void showPicture(QPixmap image);
private:
    Ui::showWidget *ui;
};

#endif // SHOWWIDGET_H
