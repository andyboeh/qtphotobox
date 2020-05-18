#ifndef ERRORWIDGET_H
#define ERRORWIDGET_H

#include <QFrame>

namespace Ui {
class errorWidget;
}

class errorWidget : public QFrame
{
    Q_OBJECT

public:

    typedef enum {
        BTN_OK,
        BTN_RETRY,
        BTN_QUIT,
        BTN_OK_RETRY,
        BTN_OK_QUIT,
        BTN_RETRY_QUIT,
        BTN_OK_RETRY_QUIT
    } errorWidgetButtons;

    explicit errorWidget(errorWidgetButtons buttons, QString message, QWidget *parent = nullptr);
    ~errorWidget();

signals:
    void errorOk();
    void errorRetry();
    void errorQuit();
private slots:
    void on_btnRetry_clicked();

    void on_btnQuit_clicked();

    void on_btnOk_clicked();

    void changeEvent(QEvent *event);
private:
    Ui::errorWidget *ui;
};

#endif // ERRORWIDGET_H
