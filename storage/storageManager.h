#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <QObject>

class QTimer;
class OrgFreedesktopDBusObjectManagerInterface;

class storageManager : public QObject
{
    Q_OBJECT
public:
    enum fileType {
        FILETYPE_FULL,
        FILETYPE_ASSEMBLED
    };
    static storageManager &getInstance();
    QString getPictureStoragePath();
    QString getRemovableStoragePath();
    QString getNextFilename(QString path, fileType type);
    QString getThumbnailStoragePath();
public slots:
    bool waitForRemovableDevice();
signals:
    void removableDeviceDetected(QString path);
private:
    storageManager();
    ~storageManager();
    QString getMountPath(QString device);
    QString mountDevice(QString device);
    QString mMountPath;
    QString mStoragePath;
    QTimer *mMountTimer;
    OrgFreedesktopDBusObjectManagerInterface *mInterface;
    QString getBaseDir();
    bool mUseRemovable;
    QStringList mIgnoreRemovable;
};

#endif //_STORAGE_MANAGER_H
