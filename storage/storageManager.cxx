#include "storageManager.h"
#include "settings.h"
#include "objectManager.h"
#include <QtDBus/QtDBus>

// Custom type for unmarhsalling byte arrays
typedef QList<unsigned char> dbus_ay;
QString dbus_ay_toString(const dbus_ay &data) {
    QString output;
    foreach(unsigned char c, data)
        if (c)  output.append(c);
    return output;
}
Q_DECLARE_METATYPE(dbus_ay)

storageManager &storageManager::getInstance()
{
    static storageManager instance;
    return instance;
}

bool storageManager::waitForRemovableDevice(bool blocking)
{
    QStringList devices;
    QStringList paths;
    QString storagePath;

    if(!QDBusConnection::systemBus().isConnected()) {
        qDebug() << "Error connecting to D-Bus";
        return false;
    }

    const QMap<QDBusObjectPath, QMap<QString, QVariantMap>> &objects = mInterface->GetManagedObjects().value();

    foreach(QDBusObjectPath objectPath, objects.keys()) {
        const QString path = objectPath.path();
        const QMap<QString, QVariantMap> object = objects.value(objectPath);

        if (!path.startsWith(QStringLiteral("/org/freedesktop/UDisks2/block_devices/"))) {
            continue;
        }

        foreach(QString key, object.keys()) {
            if(key == "org.freedesktop.UDisks2.Block") {
                QVariantMap drive_info = object.value(key);
                if(drive_info.value("IdUsage") == "filesystem" && drive_info.value("HintSystem") == false && drive_info.value("ReadOnly") == false) {
                    QString device = drive_info.value("Device").toString();
                    devices.append(device);
                }
            }
        }
    }

    foreach(QString device, devices) {
        QString shortDevice = device.mid(5);
        QString path = getMountPath(shortDevice);
        if(path.isEmpty()) {
            path = mountDevice(shortDevice);
        }
        if(!path.isEmpty()) {
            paths.append(path);
        }
    }

    if(paths.size() > 0) {
        storagePath = paths.at(0);
        return true;
    } else if(blocking) {

    }
    return false;
}

QString storageManager::getPictureStoragePath()
{
    return QString();
}

QString storageManager::getRemovableStoragePath()
{
    return QString();
}

void storageManager::waitForRemovableDevice()
{
    bool ret = waitForRemovableDevice(false);
}

storageManager::storageManager()
{
    qDBusRegisterMetaType<QMap<QString, QVariantMap>>();
    qDBusRegisterMetaType<QList<QPair<QString, QVariantMap>>>();
    qDBusRegisterMetaType<QByteArrayList>();
    qDBusRegisterMetaType<QPair<QString,QVariantMap>>();
    qDBusRegisterMetaType<QMap<QDBusObjectPath,QMap<QString,QVariantMap>>>();

    QMetaType::registerDebugStreamOperator<QList<QPair<QString, QVariantMap>>>();
    mInterface = new OrgFreedesktopDBusObjectManagerInterface("org.freedesktop.UDisks2", "/org/freedesktop/UDisks2", QDBusConnection::systemBus());
}

storageManager::~storageManager()
{
    delete mInterface;
    mInterface = nullptr;
}

QString storageManager::getMountPath(QString device)
{
    QString ret;
    QStringList mountPaths;
    QDBusInterface iface("org.freedesktop.UDisks2",
                         "/org/freedesktop/UDisks2/block_devices/" + device,
                         "org.freedesktop.DBus.Properties",
                         QDBusConnection::systemBus());
    QDBusMessage reply = iface.call("Get", "org.freedesktop.UDisks2.Filesystem", "MountPoints");
    if(reply.arguments().empty()) {
        return ret;
    }
    QVariant v = reply.arguments().first();
    QDBusArgument arg = v.value<QDBusVariant>().variant().value<QDBusArgument>();
    arg.beginArray();
    while (!arg.atEnd()) {
        dbus_ay data;
        arg >> data;
        mountPaths.append(dbus_ay_toString(data));
    }
    if(mountPaths.size() > 0) {
        ret = mountPaths.at(0);
    }
    return ret;
}

QString storageManager::mountDevice(QString device)
{
    QString ret;
    QDBusInterface iface("org.freedesktop.UDisks2",
                         "/org/freedesktop/UDisks2/block_devices/" + device,
                         "org.freedesktop.UDisks2.Filesystem",
                         QDBusConnection::systemBus());
    QDBusMessage reply = iface.call("Mount", QVariantMap());
    if(reply.arguments().size() > 0)
        ret = reply.arguments().first().toString();
    return ret;
}


