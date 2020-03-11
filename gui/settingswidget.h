#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QFrame>
#include <QMap>
#include <QString>
#include <QList>

class QLineEdit;
class QLabel;

namespace Ui {
class settingsWidget;
}

class settingsWidget : public QFrame
{
    Q_OBJECT

public:
    explicit settingsWidget(QFrame *parent = nullptr);
    ~settingsWidget();

private:
    void loadDefaultsFromSettings();
    void loadFromSettings();
    void saveToSettings();

private slots:
    void on_btnSave_clicked();

    void on_btnCancel_clicked();

    void on_btnRestoreDefaults_clicked();

    void on_chkEnableshow_stateChanged(int arg1);

    void on_chkShowFullscreen_stateChanged(int arg1);

    void on_chkEnableArchive_stateChanged(int arg1);

    void on_chkEnableGPIO_stateChanged(int arg1);

    void on_chkWaitRemovable_stateChanged(int arg1);

    void on_chkEnablePrinting_stateChanged(int arg1);

    void on_btnBackground_clicked();

    void on_chkStartFullscreen_stateChanged(int arg1);

    void on_spinNumPictures_valueChanged(int arg1);

private:
    Ui::settingsWidget *ui;
    QMap<QString, QString> mStyleMapping;
    QMap<QString, QString> mLanguageMapping;
    QMap<int, QString> mRotationMapping;
    QMap<QString, QString> mPrinterBackendMapping;
    QMap<QString, QString> mCameraBackendMapping;
    QList<QLabel*> mFilterLabels;
    QList<QLineEdit*> mFilterEdits;
};

#endif // SETTINGSWIDGET_H
