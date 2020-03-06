#ifndef ARCHIVEWIDGET_H
#define ARCHIVEWIDGET_H

#include <QFrame>

namespace Ui {
class archiveWidget;
}

class picturedetailWidget;

class archiveWidget : public QFrame
{
    Q_OBJECT

public:
    explicit archiveWidget(QWidget *parent = nullptr);
    ~archiveWidget();
signals:
    void printAssembledPicture(QString filename, int copies);
private slots:
    void on_btnClose_clicked();
    void printArchivePicture(QString filename, int copies);
    void on_listView_clicked(const QModelIndex &index);

private:
    Ui::archiveWidget *ui;
    QString mBasepath;
    QString mBasename;
    QString mStoragepath;
    picturedetailWidget *mDetailWidget;
};

#endif // ARCHIVEWIDGET_H
