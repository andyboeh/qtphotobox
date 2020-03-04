#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QFrame>

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

private:
    Ui::settingsWidget *ui;
};

#endif // SETTINGSWIDGET_H
