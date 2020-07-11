#include "camera_gphoto2.h"
#include "settings.h"
#include <QDebug>
#include <QFile>
#include <QSettings>

CameraGphoto2::CameraGphoto2()
{
    mContext = nullptr;
    mCamera = nullptr;
    mActive = false;
}

CameraGphoto2::~CameraGphoto2()
{
    qDebug() << "~CameraGphoto2";
    if(mCamera) {
        setShutdownConfig();
        gp_camera_exit(mCamera, mContext);
        gp_camera_free(mCamera);
    }
}

bool CameraGphoto2::initCamera()
{
    int ret;
    if(!mContext) {
        mContext = gp_context_new();
        /*
        gp_context_set_error_func(mContext, ctx_error_func, NULL);
        gp_context_set_status_func(mContext, ctx_status_func, NULL);
        */
    }
    if(!mCamera) {
        ret = gp_camera_new(&mCamera);
        if (ret != GP_OK) {
            qDebug() << "Error initializing camera.";
            gp_camera_free(mCamera);
            mCamera = nullptr;
            return false;
        }
        ret = gp_camera_init(mCamera, mContext);
        if(ret != GP_OK) {
            qDebug() << "Error initializing camera.";
            gp_camera_free(mCamera);
            mCamera = nullptr;
            return false;
        }
        QMap<QString, QVariant> config = getConfig();
        if(config.contains("cameramodel")) {
            QString filename = config.value("cameramodel").toString().toLower().remove(" ") + ".cfg";
            pbSettings &pbs = pbSettings::getInstance();
            filename = pbs.getConfigPath() + filename;
            qDebug() << "Looking for file: " << filename;
            QFile cfg(filename);
            if(cfg.exists()) {
                qDebug() << "Loading config from file.";
                QSettings settings(filename, QSettings::IniFormat);
                QStringList sections = settings.childGroups();
                if(sections.contains("startup")) {
                    settings.beginGroup("startup");
                    foreach(QString key, settings.childKeys()) {
                        mStartupConfig.insert(key, settings.value(key));
                    }
                    settings.endGroup();
                }
                if(sections.contains("shutdown")) {
                    settings.beginGroup("shutdown");
                    foreach(QString key, settings.childKeys()) {
                        mShutdownConfig.insert(key, settings.value(key));
                    }
                    settings.endGroup();
                }
                if(sections.contains("active")) {
                    settings.beginGroup("active");
                    foreach(QString key, settings.childKeys()) {
                        mActiveConfig.insert(key, settings.value(key));
                    }
                    settings.endGroup();
                }
                if(sections.contains("idle")) {
                    settings.beginGroup("idle");
                    foreach(QString key, settings.childKeys()) {
                        mIdleConfig.insert(key, settings.value(key));
                    }
                    settings.endGroup();
                }
                if(sections.contains("capture")) {
                    settings.beginGroup("capture");
                    foreach(QString key, settings.childKeys()) {
                        mCaptureConfig.insert(key, settings.value(key));
                    }
                }
            }
        }
        setStartupConfig();
    }
    return true;
}

QPixmap CameraGphoto2::getPreviewImage()
{
    QPixmap image;
    int ret;
    CameraFile *file = NULL;
    const char *data;
    unsigned long size;

    ret = gp_file_new(&file);
    if(ret != GP_OK) {
        qDebug() << "gp_file_new failed.";
        goto out;
    }

    ret = gp_camera_capture_preview(mCamera, file, mContext);
    if(ret != GP_OK) {
        qDebug() << "gp_camera_capture_preview failed.";
        goto out;
    }

    ret = gp_file_get_data_and_size(file, &data, &size);
    if(ret != GP_OK) {
        qDebug() << "gp_file_get_data_and_size failed.";
        goto out;
    }

    image.loadFromData((const unsigned char *)data, (unsigned int)size);

out:
    gp_file_free(file);
    return image;
}

QPixmap CameraGphoto2::getCaptureImage()
{
    QPixmap image;
    unsigned long size;
    int ret;
    const char *data;
    CameraFile *file = NULL;
    CameraFilePath camera_file_path;

    strcpy(camera_file_path.folder, "/");
    strcpy(camera_file_path.name, "foo.jpg");

    setCaptureConfig();

    ret = gp_camera_capture(mCamera, GP_CAPTURE_IMAGE, &camera_file_path, mContext);
    if(ret != GP_OK) {
        qDebug() << "gp_camera_capture failed.";
        goto out;
    }

    ret = gp_file_new(&file);
    if(ret != GP_OK) {
        qDebug() << "gp_file_new failed.";
        goto out;
    }

    ret = gp_camera_file_get(mCamera, camera_file_path.folder, camera_file_path.name,
                             GP_FILE_TYPE_NORMAL, file, mContext);
    if(ret != GP_OK) {
        qDebug() << "gp_camera_file_get failed.";
        goto out;
    }

    ret = gp_file_get_data_and_size(file, &data, &size);
    if(ret != GP_OK) {
        qDebug() << "gp_file_get_data_and_size failed.";
        goto out;
    }

    image.loadFromData((const unsigned char *)data, (unsigned int)size);

    ret = gp_camera_file_delete(mCamera, camera_file_path.folder, camera_file_path.name, mContext);
    if(ret != GP_OK) {
        qDebug() << "gp_camera_file_delete failed.";
        goto out;
    }

out:
    return image;
}

void CameraGphoto2::setIdle()
{
    foreach(QString key, mIdleConfig.keys()) {
        setConfigValue(key, mIdleConfig.value(key));
    }
}

void CameraGphoto2::setActive()
{
    foreach(QString key, mActiveConfig.keys()) {
        setConfigValue(key, mActiveConfig.value(key));
    }
}

bool CameraGphoto2::setConfigValue(QString key, QVariant val)
{
    CameraWidget *widget = NULL;
    CameraWidget *child = NULL;
    CameraWidgetType type;
    int	ret;
    bool retVal = false;
    qDebug() << "setConfigValueString: " << key << " = " << val;

    ret = gp_camera_get_config (mCamera, &widget, mContext);
    if (ret < GP_OK) {
        qDebug() << "camera_get_config failed.";
        return false;
    }
    ret = gp_widget_get_child_by_name (widget, key.toStdString().c_str(), &child);
    if (ret < GP_OK) {
        ret = gp_widget_get_child_by_label (widget, key.toStdString().c_str(), &child);
    }
    if (ret < GP_OK) {
        qDebug() << "lookup widget failed.";
        goto out;
    }

    /* This type check is optional, if you know what type the label
     * has already. If you are not sure, better check. */
    ret = gp_widget_get_type (child, &type);
    if (ret < GP_OK) {
        qDebug() << "widget get type failed.";
        goto out;
    }

    switch (type) {
        case GP_WIDGET_MENU:
        case GP_WIDGET_RADIO:
        case GP_WIDGET_TEXT:
        /* This is the actual set call. Note that we keep
         * ownership of the string and have to free it if necessary.
         */
        ret = gp_widget_set_value(child, val.toString().toStdString().c_str());
        if (ret < GP_OK) {
            qDebug() << "could not set widget value.";
            goto out;
        }
        break;
    case GP_WIDGET_TOGGLE:
    {
        int value = val.toInt();
        /* This is the actual set call. Note that we keep
         * ownership of the string and have to free it if necessary.
         */
        ret = gp_widget_set_value(child, &value);
        if (ret < GP_OK) {
            qDebug() << "could not set widget value.";
            goto out;
        }
        break;
    }
    default:
        qDebug() <<  "widget has bad type.";
        goto out;
    }

    ret = gp_camera_set_single_config (mCamera, key.toStdString().c_str(), child, mContext);
    if (ret != GP_OK) {
        /* This stores it on the camera again */
        ret = gp_camera_set_config (mCamera, widget, mContext);
        if (ret < GP_OK) {
            qDebug() << "camera_set_config failed.";
            goto out;
        }
    }
    retVal = true;
    qDebug() << "Success";
out:
    gp_widget_free (widget);
    return retVal;
}

QMap<QString, QVariant> CameraGphoto2::getChildValue(CameraWidget *child) {
    int ret;
    const char *temp;
    CameraWidgetType type;
    QMap<QString, QVariant> config;

    ret = gp_widget_get_name(child, &temp);
    if(ret != GP_OK) {
        return config;
    }
    ret = gp_widget_get_type(child, &type);
    if(ret != GP_OK) {
        return config;
    }
    switch(type) {
    case GP_WIDGET_MENU:
    case GP_WIDGET_RADIO:
    case GP_WIDGET_TEXT:
    {
        char *val;
        ret = gp_widget_get_value(child, &val);
        if(ret != GP_OK)
            return config;
        config.insert(QString(temp), val);
        break;
    }
    default:
        break;
    }
    return config;
}

QMap<QString, QVariant> CameraGphoto2::getConfigRecursively(CameraWidget *widget) {
    QMap<QString, QVariant> config;
    int ret;
    int count;

    ret = gp_widget_count_children(widget);
    if(ret < 0) {
        return config;
    }
    if(ret == 0) {
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
        config = config.unite(getChildValue(widget));
#else
        config.insert(getChildValue(widget));
#endif
    }
    count = ret;
    for(int i=0; i<count; i++) {
        CameraWidget *child;
        ret = gp_widget_get_child(widget, i, &child);
        if(ret == GP_OK) {
            if(gp_widget_count_children(child) > 0) {
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
                config = config.unite(getConfigRecursively(child));
#else
                config.insert(getConfigRecursively(child));
#endif
            } else {
#if QT_VERSION < QT_VERSION_CHECK(5,15,0)
                config = config.unite(getChildValue(child));
#else
                config.insert(getChildValue(child));
#endif
            }
        }
    }
    return config;
}

QMap<QString, QVariant> CameraGphoto2::getConfig()
{
    QMap<QString, QVariant> config;
    int ret;
    CameraWidget *widget = NULL;

    ret = gp_camera_get_config(mCamera, &widget, mContext);
    if(ret != GP_OK) {
        return config;
    }
    config = getConfigRecursively(widget);

    /*
    foreach(QString key, config.keys()) {
        qDebug() << key << " = " << config[key].toString();
    }
    */
    gp_widget_free(widget);
    return config;
}

QVariant CameraGphoto2::getConfig(QString key)
{
    QMap<QString, QVariant> config = getConfig();
    return config.value(key);
}

void CameraGphoto2::setStartupConfig()
{
    foreach(QString key, mStartupConfig.keys()) {
        setConfigValue(key, mStartupConfig.value(key));
    }
}

void CameraGphoto2::setShutdownConfig()
{
    foreach(QString key, mShutdownConfig.keys()) {
        setConfigValue(key, mShutdownConfig.value(key));
    }
}

void CameraGphoto2::setCaptureConfig()
{
    foreach(QString key, mCaptureConfig.keys()) {
        setConfigValue(key, mCaptureConfig.value(key));
    }
}
