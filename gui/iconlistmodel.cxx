#include "iconlistmodel.h"

#include <QGuiApplication>
#include <QDir>
#include <QIcon>
#include <QPalette>

iconListModel::iconListModel(QObject *parent)
    : QAbstractListModel(parent), mFileCount(0)
{}

int iconListModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : mFileCount;
}

QVariant iconListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= mFileList.size() || index.row() < 0) {
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        QString name = mFileList.at(index.row());
        return name.remove(mBasename);
    } else if (role == Qt::DecorationRole) {
        return QIcon(mBaseDir + QDir::separator() + mFileList.at(index.row()));
    } else if (role == Qt::BackgroundRole) {
        int batch = (index.row() / 100) % 2;
        if (batch == 0)
            return qApp->palette().base();
        else
            return qApp->palette().alternateBase();
    }
    return QVariant();
}

bool iconListModel::canFetchMore(const QModelIndex &parent) const
{
    if (parent.isValid())
        return false;
    return (mFileCount < mFileList.size());
}

void iconListModel::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
        return;
    int remainder = mFileList.size() - mFileCount;
    int itemsToFetch = qMin(100, remainder);

    if (itemsToFetch <= 0)
        return;

    beginInsertRows(QModelIndex(), mFileCount, mFileCount + itemsToFetch - 1);

    mFileCount += itemsToFetch;

    endInsertRows();

    emit numberPopulated(itemsToFetch);
}

void iconListModel::setDirPath(const QString &path, const QStringList &filter, const QString &basename)
{
    QDir dir(path);

    beginResetModel();
    mFileList = dir.entryList(filter);
    mFileCount = 0;
    mBaseDir = path;
    mBasename = basename;
    endResetModel();
}
