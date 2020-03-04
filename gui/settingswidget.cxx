#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "statemachine.h"
#include "settings.h"

settingsWidget::settingsWidget(QFrame *parent) :
    QFrame(parent),
    ui(new Ui::settingsWidget)
{
    ui->setupUi(this);
    loadFromSettings();
}

settingsWidget::~settingsWidget()
{
    delete ui;
}

void settingsWidget::loadDefaultsFromSettings()
{
    pbSettings &pbs = pbSettings::getInstance();
    pbs.initDefaultConfg();
    loadFromSettings();
}

void settingsWidget::loadFromSettings()
{
    pbSettings &pbs = pbSettings::getInstance();
    QString path = pbs.getConfigPath();
    path += "settings.ini";
    pbs.mergeConfigFile(path);
}

void settingsWidget::saveToSettings()
{
    pbSettings &pbs = pbSettings::getInstance();
    QString path = pbs.getConfigPath();
    path += "settings.ini";
    pbs.saveConfigFile(path);
}

void settingsWidget::on_btnSave_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("init");
}

void settingsWidget::on_btnCancel_clicked()
{
    StateMachine &sm = StateMachine::getInstance();
    sm.triggerState("init");
}

void settingsWidget::on_btnRestoreDefaults_clicked()
{

}
