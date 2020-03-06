#ifndef PICTUREDETAILWIDGET_H
#define PICTUREDETAILWIDGET_H

#include <QFrame>
#include <QPixmap>

namespace Ui {
class picturedetailWidget;
}

class picturedetailWidget : public QFrame
{
    Q_OBJECT

public:
    explicit picturedetailWidget(QString filename, QWidget *parent = nullptr);
    ~picturedetailWidget();


private slots:
    void paintEvent(QPaintEvent *event);
    void on_btnPrint_clicked();

    void on_btnClose_clicked();

    void on_sliderCopies_valueChanged(int value);
signals:
    void printArchivePicture(QString filename, int copies);
private:
    Ui::picturedetailWidget *ui;
    QString mFilename;
    int mWidth;
    int mHeight;
    QPixmap mScaledImage;
};

#endif // PICTUREDETAILWIDGET_H
