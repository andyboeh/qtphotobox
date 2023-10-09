#include "settings.h"
#include <QSettings>
#include <QFile>

pbSettings &
pbSettings::getInstance() {
    static pbSettings instance;
    return instance;
}

void pbSettings::setConfigPath(QString path)
{
    mConfigDir = path;
}

QString pbSettings::getConfigPath()
{
    return mConfigDir;
}

pbSettings::pbSettings()
{
    mSettings.clear();
}

pbSettings::~pbSettings()
{

}

void pbSettings::mergeConfigFile(QString path)
{
    QSettings settings(path, QSettings::IniFormat);
    QStringList sections = settings.childGroups();
    foreach(QString section, sections) {
        settings.beginGroup(section);
        QStringList keys = settings.childKeys();
        foreach(QString key, keys) {
            QVariant value = settings.value(key);
            if(mSettings.contains(section)) {
                if(mSettings[section].contains(key)) {
                    mSettings[section][key] = value;
                } else {
                    mSettings[section].insert(key, value);
                }
            } else {
                QMap<QString, QVariant> temp;
                temp.insert(key, value);
                mSettings.insert(section, temp);
            }
        }
        settings.endGroup();
    }
}

void pbSettings::saveConfigFile(QString path)
{
    QFile settingsFile(path);
    if(settingsFile.exists())
        settingsFile.remove();

    QSettings settings(path, QSettings::IniFormat);
    foreach(QString section, mSettings.keys()) {
        settings.beginGroup(section);
        foreach(QString key, mSettings[section].keys()) {
            settings.setValue(key, mSettings[section][key]);
        }
        settings.endGroup();
    }

    settings.sync();
}

void pbSettings::initDefaultConfg()
{
    mSettings.clear();

    // Gui
    QMap<QString, QVariant> guiSettings;
    QMap<QString, QVariant> cameraSettings;
    QMap<QString, QVariant> gpioSettings;
    QMap<QString, QVariant> printerSettings;
    QMap<QString, QVariant> qtPhotoboxSettings;
    QMap<QString, QVariant> pictureSettings;
    QMap<QString, QVariant> storageSettings;
    QMap<QString, QVariant> archiveSettings;
    QMap<QString, QVariant> showSettings;
    QMap<QString, QVariant> screensaverSettings;

    guiSettings.insert("fullscreen", false);
    guiSettings.insert("width", 1024);
    guiSettings.insert("height", 600);
    guiSettings.insert("hide_cursor", false);
    guiSettings.insert("style", "default");
    guiSettings.insert("direct_start", false);
    guiSettings.insert("language", "english");
    guiSettings.insert("shutdown", false);

    screensaverSettings.insert("enable", true);
    screensaverSettings.insert("timeout", 60);
    screensaverSettings.insert("text1", "QtPhotobox");
    screensaverSettings.insert("text2", "by Andreas Böhler");
    screensaverSettings.insert("text3", "www.aboehler.at");

    cameraSettings.insert("capturebackend", "gphoto2");
    cameraSettings.insert("capturename", "UVC Camera");
    cameraSettings.insert("capturerotation", 0);
    cameraSettings.insert("captureflip", false);
    cameraSettings.insert("previewbackend", "gphoto2");
    cameraSettings.insert("previewname", "UVC Camera");
    cameraSettings.insert("previewrotation", 0);
    cameraSettings.insert("previewflip", true);
    cameraSettings.insert("fps", 0);
    cameraSettings.insert("previewportrait", false);
    cameraSettings.insert("captureportrait", false);

    gpioSettings.insert("enable", true);
    gpioSettings.insert("exit_pin", 24);
    gpioSettings.insert("trigger_pin", 23);
    gpioSettings.insert("af_lamp_pin", 16);
    gpioSettings.insert("af_lamp_pwm_value", 50);
    gpioSettings.insert("idle_lamp_pin", 12);
    gpioSettings.insert("idle_lamp_pwm_value", 50);

    printerSettings.insert("enable", true);
    printerSettings.insert("autoprint", false);
    printerSettings.insert("allowprinting", true);
    printerSettings.insert("enforcepassword", false);
    printerSettings.insert("max_copies", 10);
    printerSettings.insert("width", 148);
    printerSettings.insert("height", 100);
    printerSettings.insert("backend", "selphy");
    printerSettings.insert("ip", "192.168.123.123");

    qtPhotoboxSettings.insert("show_preview", true);
    qtPhotoboxSettings.insert("greeter_time", 3);
    qtPhotoboxSettings.insert("countdown_time", 8);
    qtPhotoboxSettings.insert("display_time", 5);

    pictureSettings.insert("num_pictures", 4);
    pictureSettings.insert("filters_picture1", "scale:width=47%,move:top=30:left=30");
    pictureSettings.insert("filters_picture2", "scale:width=47%,move:top=30:right=30");
    pictureSettings.insert("filters_picture3", "scale:width=47%,move:bottom=30:left=30");
    pictureSettings.insert("filters_picture4", "scale:width=47%,move:bottom=30:right=30");
    pictureSettings.insert("size_x", 3496);
    pictureSettings.insert("size_y", 2362);
    pictureSettings.insert("background", "");

    storageSettings.insert("basedir", "%Y-%m-%d");
    storageSettings.insert("basename", "qtphotobox");
    storageSettings.insert("keep_pictures", false);
    storageSettings.insert("wait_removable", false);
    storageSettings.insert("ignore_removable", "/dev/mmcblk0p1,/dev/mmcblk0p2");

    archiveSettings.insert("enable", true);
    archiveSettings.insert("allow_reprint", true);

    showSettings.insert("enable", true);
    showSettings.insert("swap_screens", false);
    showSettings.insert("display_time", 5);
    showSettings.insert("random", false);
    showSettings.insert("fullscreen", false);
    showSettings.insert("width", 1024);
    showSettings.insert("height", 600);
    showSettings.insert("footertext", "QtPhotobox");
    showSettings.insert("full_images", false);

    mSettings.insert("gui", guiSettings);
    mSettings.insert("qtphotobox", qtPhotoboxSettings);
    mSettings.insert("printer", printerSettings);
    mSettings.insert("camera", cameraSettings);
    mSettings.insert("gpio", gpioSettings);
    mSettings.insert("picture", pictureSettings);
    mSettings.insert("storage", storageSettings);
    mSettings.insert("archive", archiveSettings);
    mSettings.insert("show", showSettings);
    mSettings.insert("screensaver", screensaverSettings);
}

QString pbSettings::get(QString section, QString key)
{
    return mSettings[section][key].toString();
}

bool pbSettings::set(QString section, QString key, QString value)
{
    return setVariant(section, key, value);
}

int pbSettings::getInt(QString section, QString key)
{
    return mSettings[section][key].toInt();
}

bool pbSettings::setInt(QString section, QString key, int value)
{
    return setVariant(section, key, value);
}

bool pbSettings::getBool(QString section, QString key)
{
    return mSettings[section][key].toBool();
}

bool pbSettings::setBool(QString section, QString key, bool value)
{
    return setVariant(section, key, value);
}

bool pbSettings::setVariant(QString section, QString key, QVariant value)
{
    if(!mSettings.contains(section)) {
        QMap<QString, QVariant> temp;
        temp.insert(key, value);
        mSettings.insert(section, temp);
    } else {
        if(!mSettings[section].contains(key)) {
            mSettings[section].insert(key, value);
        } else {
            mSettings[section][key] = value;
        }
    }
    return true;
}
