#ifndef _ICONLISTMODEL_H
#define _ICONLISTMODEL_H

#include <QAbstractListModel>
#include <QStringList>

class iconListModel : public QAbstractListModel
{
    Q_OBJECT

public:
    iconListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

signals:
    void numberPopulated(int number);

public slots:
    void setDirPath(const QString &path, const QStringList &filter = QStringList(), const QString &basename = "");

protected:
    bool canFetchMore(const QModelIndex &parent) const override;
    void fetchMore(const QModelIndex &parent) override;

private:
    QStringList mFileList;
    QString mBaseDir;
    QString mBasename;
    int mFileCount;
};

#endif // FILELISTMODEL_H
