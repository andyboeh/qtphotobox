#ifndef REVIEWWIDGET_H
#define REVIEWWIDGET_H

#include <QFrame>
#include <QPixmap>

namespace Ui {
class reviewWidget;
}

class QTimer;

class reviewWidget : public QFrame
{
    Q_OBJECT

public:
    explicit reviewWidget(QPixmap image, QFrame *parent = nullptr);
    ~reviewWidget();
public slots:
    void timeout(void);
private slots:
    void paintEvent(QPaintEvent *event);
    void changeEvent(QEvent *event);
private:
    Ui::reviewWidget *ui;
    QTimer *mTimer;
    QPixmap mImage;
    QPixmap mScaledImage;
    int mWidth;
    int mHeight;
};

#endif // REVIEWWIDGET_H
