#ifndef _STORAGE_MANAGER_H
#define _STORAGE_MANAGER_H

#include <QObject>

class OrgFreedesktopDBusObjectManagerInterface;

class storageManager : public QObject
{
    Q_OBJECT
public:
    static storageManager &getInstance();
    bool waitForRemovableDevice(bool blocking);
    QString getPictureStoragePath();
    QString getRemovableStoragePath();
public slots:
    void waitForRemovableDevice();
signals:
    void removeDeviceDetected(bool result);
private:
    storageManager();
    ~storageManager();
    QString getMountPath(QString device);
    QString mountDevice(QString device);
    OrgFreedesktopDBusObjectManagerInterface *mInterface;
};

#endif //_STORAGE_MANAGER_H
