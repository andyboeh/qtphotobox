#ifndef _SETTINGS_H
#define _SETTINGS_h

#include <QString>
#include <QMap>
#include <QVariant>

class pbSettings {
public:
    static pbSettings& getInstance();

    void setConfigPath(QString path);
    QString getConfigPath(void);

    void mergeConfigFile(QString path);
    void saveConfigFile(QString path);

    void initDefaultConfg();

    QString get(QString section, QString key);
    bool set(QString section, QString key, QString value);

    int getInt(QString section, QString key);
    bool setInt(QString section, QString key, int value);

    bool getBool(QString section, QString key);
    bool setBool(QString section, QString key, bool value);

private:
    Q_DISABLE_COPY(pbSettings);
    pbSettings();
    ~pbSettings();
    QMap<QString, QMap<QString, QVariant> > mSettings;
    QString mConfigDir;

    bool setVariant(QString section, QString key, QVariant value);
};

#endif //_SETTINGS_H
