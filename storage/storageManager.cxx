#include "storageManager.h"
#include "settings.h"
#include "objectManager.h"
#include <QtDBus/QtDBus>
#include <QTimer>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDateTime>

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

bool storageManager::waitForRemovableDevice()
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
                    if(!mIgnoreRemovable.contains(device)) {
                        devices.append(device);
                    }
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
        if(mMountTimer->isActive())
            mMountTimer->stop();
        if(mUseRemovable) {
            mStoragePath = storagePath + QDir::separator() + getBaseDir();
            qDebug() << "Using storage Path: " << mStoragePath;
        }
        emit removableDeviceDetected(storagePath);
        return true;
    } else {
        if(!mMountTimer->isActive()) {
            mMountTimer->start();
        }
    }
    return false;
}

QString storageManager::getPictureStoragePath()
{
    return mStoragePath;
}

QString storageManager::getThumbnailStoragePath()
{
    return mStoragePath + QDir::separator() + "thumbs";
}

QString storageManager::getBaseDir() {
    pbSettings &pbs = pbSettings::getInstance();
    QString basedir = pbs.get("storage", "basedir");
    QDateTime currTime = QDateTime::currentDateTime();

    basedir = basedir.replace("%Y", currTime.date().toString("yyyy"));
    basedir = basedir.replace("%m", currTime.date().toString("MM"));
    basedir = basedir.replace("%d", currTime.date().toString("dd"));
    return basedir;
}

QString storageManager::getRemovableStoragePath()
{
    return mMountPath;
}

QString storageManager::getNextFilename(QString path, storageManager::fileType type)
{
    int number = 0;

    QDir dir(path);
    dir.mkpath(path);
    dir.mkpath(path + QDir::separator() + "thumbs");
    pbSettings &pbs = pbSettings::getInstance();
    QString basename = pbs.get("storage", "basename");
    if(type == FILETYPE_FULL) {
        basename = basename + "-full-";
    } else {
        basename = basename + "-assembled-";
    }
    QStringList filters;
    filters.append(basename + "*");
    QStringList files = dir.entryList(filters);
    if(files.size() > 0) {
        files.sort();
        bool ok;
        number = files.last().remove(basename).section(".", 0, 0).toInt(&ok);
        if(!ok)
            number = 0;
        else
            number++;
    }
    QString fn = QString(basename + "%1.jpg").arg(number, 4, 10, QChar('0'));
    qDebug() << "Returning save filename" << fn;
    return fn;
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
    // If requested, poll every tsecond for new removable devices.
    mMountTimer = new QTimer();
    mMountTimer->setSingleShot(false);
    mMountTimer->setInterval(1000);
    connect(mMountTimer, SIGNAL(timeout()), this, SLOT(waitForRemovableDevice()));

    QString basedir = getBaseDir();

    QFileInfo baseinfo(basedir);
    if(baseinfo.isRelative()) {
        QString picturePath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        qDebug() << picturePath;
        if(!picturePath.isEmpty()) {
            mStoragePath = picturePath + QDir::separator() + basedir;
        }
    } else {
        mStoragePath = basedir;
    }
    qDebug() << "Using storage path: " << mStoragePath;

    pbSettings &pbs = pbSettings::getInstance();
    mUseRemovable = pbs.getBool("storage", "wait_removable");
    mIgnoreRemovable = pbs.get("storage", "ignore_removable").split(",");
    mIgnoreRemovable.removeAll("");
}

storageManager::~storageManager()
{
    mMountTimer->stop();
    mMountTimer->deleteLater();
    mInterface->deleteLater();
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


