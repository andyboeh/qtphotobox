#ifndef PASSWORDWIDGET_H
#define PASSWORDWIDGET_H

#include <QFrame>
#include <QList>

namespace Ui {
class passwordWidget;
}

class passwordWidget : public QFrame
{
    Q_OBJECT

public:
    explicit passwordWidget(QWidget *parent = nullptr);
    explicit passwordWidget(QString filename, int copies, QWidget *parent = nullptr);
    ~passwordWidget();

signals:
    void printArchivePicture(QString filename, int copies);
    void printPicture();
    void cancelled();

private slots:
    void changeEvent(QEvent *event);
    void on_btnNum1_clicked();

    void on_btnNum2_clicked();

    void on_btnNum3_clicked();

    void on_btnNum4_clicked();

    void on_btnNum5_clicked();

    void on_btnNum6_clicked();

    void on_btnNum7_clicked();

    void on_btnNum8_clicked();

    void on_btnNum9_clicked();

    void on_btnNum0_clicked();

    void on_btnCancel_clicked();

private:
    void checkPassword();
    Ui::passwordWidget *ui;
    QString mFilename;
    int mCopies;
    QList<int> mDigits;
    QList<int> mPassword;
    int mCurrentDigit;
    void loadSettings();
};

#endif // PASSWORDWIDGET_H
