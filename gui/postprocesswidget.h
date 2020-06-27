#ifndef POSTPROCESSWIDGET_H
#define POSTPROCESSWIDGET_H

#include <QFrame>

namespace Ui {
class postprocessWidget;
}

class passwordWidget;

class postprocessWidget : public QFrame
{
    Q_OBJECT

public:
    explicit postprocessWidget(QWidget *parent = nullptr);
    ~postprocessWidget();

signals:
    void startPrintJob(int numcopies);
private slots:
    void on_btnPrint_clicked();
    void on_btnStartOver_clicked();
    void on_sliderNumCopies_valueChanged(int value);

    void changeEvent(QEvent *event);
    void printFromPasswordDialog();
    void passwordDialogClosed();
private:
    Ui::postprocessWidget *ui;
    passwordWidget *mPasswordWidget;
};

#endif // POSTPROCESSWIDGET_H
