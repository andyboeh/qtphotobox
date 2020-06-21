#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <QObject>

class QTimer;
#ifdef BUILD_WAIT_USB
class OrgFreedesktopDBusObjectManagerInterface;
#endif

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
#ifdef BUILD_WAIT_USB
public slots:
    bool waitForRemovableDevice();
#endif
#ifdef BUILD_WAIT_USB
signals:
    void removableDeviceDetected(QString path);
#endif
private:
    storageManager();
    ~storageManager();
#ifdef BUILD_WAIT_USB
    QString getMountPath(QString device);
    QString mountDevice(QString device);
    OrgFreedesktopDBusObjectManagerInterface *mInterface;
#endif
    QString mMountPath;
    QString mStoragePath;
    QTimer *mMountTimer;
    QString getBaseDir();
    bool mUseRemovable;
    QStringList mIgnoreRemovable;
};

#endif //_STORAGE_MANAGER_H
