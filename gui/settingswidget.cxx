#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "statemachine.h"
#include "settings.h"
#include <QLineEdit>
#include <QLabel>
#include <QSpacerItem>
#include <QFileDialog>
#include <QDebug>

settingsWidget::settingsWidget(QFrame *parent) :
    QFrame(parent),
    ui(new Ui::settingsWidget)
{
    ui->setupUi(this);
    mStyleMapping.insert("dark-1024x600", "Dark (1024x600)");
    mStyleMapping.insert("dark-800x600", "Dark (800x600)");
    mStyleMapping.insert("pastel-1024x600", "Pastel (1024x600)");
    mStyleMapping.insert("pastel-800x600", "Pastel (800x600)");
    mStyleMapping.insert("default", "Default (System)");

    mLanguageMapping.insert("english", "English");
    mLanguageMapping.insert("german", "Deutsch");

    mRotationMapping.insert(0, "0º");
    mRotationMapping.insert(90, "90º");
    mRotationMapping.insert(180, "180º");
    mRotationMapping.insert(270, "270º");

    mCameraBackendMapping.insert("gphoto2", "Gphoto2");
    mCameraBackendMapping.insert("dummy", "Dummy");

    mPrinterBackendMapping.insert("selphy", "Selphy (WiFi)");
    mPrinterBackendMapping.insert("selphyusb", "Selphy (USB)");
    mPrinterBackendMapping.insert("cups", "CUPS");

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
    if(pbs.getBool("gui", "hide_cursor"))
        ui->chkHideCursor->setCheckState(Qt::Checked);
    else
        ui->chkHideCursor->setCheckState(Qt::Unchecked);

    if(pbs.getBool("gui", "direct_start"))
        ui->chkStartDirectly->setCheckState(Qt::Checked);
    else
        ui->chkStartDirectly->setCheckState(Qt::Unchecked);

    if(pbs.getBool("gui", "fullscreen")) {
        ui->chkStartFullscreen->setCheckState(Qt::Checked);
        on_chkStartFullscreen_stateChanged(Qt::Checked);
    } else {
        ui->chkStartFullscreen->setCheckState(Qt::Unchecked);
        on_chkStartFullscreen_stateChanged(Qt::Unchecked);
    }

    ui->spinHeight->setValue(pbs.getInt("gui", "height"));
    ui->spinWidth->setValue(pbs.getInt("gui", "width"));

    QString style = pbs.get("gui", "style");
    qDebug() << style;
    qDebug() << mStyleMapping.value(style);
    ui->cmbStyle->setCurrentText(mStyleMapping.value(style));

    QString language = pbs.get("gui", "language");
    ui->cmbLanguage->setCurrentText(mLanguageMapping.value(language));

    if(pbs.getBool("archive", "enable")) {
        ui->chkEnableArchive->setChecked(Qt::Checked);
        on_chkEnableArchive_stateChanged(Qt::Checked);
    } else {
        ui->chkEnableArchive->setChecked(Qt::Unchecked);
        on_chkEnableArchive_stateChanged(Qt::Unchecked);
    }

    if(pbs.getBool("archive", "allow_reprint"))
        ui->chkAllowReprint->setChecked(Qt::Checked);
    else
        ui->chkAllowReprint->setChecked(Qt::Unchecked);

    QString camBackend = pbs.get("camera", "backend");
    ui->cmbBackend->setCurrentText(mCameraBackendMapping.value(camBackend));
    ui->spinFps->setValue(pbs.getInt("camera", "fps"));
    int rot = pbs.getInt("camera", "rotation");
    ui->cmbRotation->setCurrentText(mRotationMapping.value(rot));

    if(pbs.getBool("gpio", "enable")) {
        ui->chkEnableGPIO->setChecked(Qt::Checked);
        on_chkEnableGPIO_stateChanged(Qt::Checked);
    } else {
        ui->chkEnableGPIO->setChecked(Qt::Unchecked);
        on_chkEnableGPIO_stateChanged(Qt::Unchecked);
    }

    ui->spinAfLamp->setValue(pbs.getInt("gpio", "af_lamp_pin"));
    ui->spinAfValue->setValue(pbs.getInt("gpio", "af_lamp_pwm_value"));
    ui->spinIdleLamp->setValue(pbs.getInt("gpio", "idle_lamp_pin"));
    ui->spinIdleValue->setValue(pbs.getInt("gpio", "idle_lamp_pwm_value"));
    ui->spinTrigger->setValue(pbs.getInt("gpio", "trigger_pin"));
    ui->spinExit->setValue(pbs.getInt("gpio", "exit_pin"));

    ui->spinCountdown->setValue(pbs.getInt("qtphotobox", "countdown_time"));
    ui->spinDisplay->setValue(pbs.getInt("qtphotobox", "display_time"));
    ui->spinGreeter->setValue(pbs.getInt("qtphotobox", "greeter_time"));
    ui->spinPostprocessing->setValue(pbs.getInt("qtphotobox", "postprocess_time"));
    if(pbs.getBool("qtphotobox", "show_preview"))
        ui->chkPreview->setChecked(Qt::Checked);
    else
        ui->chkPreview->setChecked(Qt::Unchecked);

    ui->editBackground->setText(pbs.get("picture", "background"));
    ui->spinNumPictures->setValue(pbs.getInt("picture", "num_pictures"));
    ui->spinPictureX->setValue(pbs.getInt("picture", "size_x"));
    ui->spinPictureY->setValue(pbs.getInt("picture", "size_y"));

    if(pbs.getBool("printer", "enable")) {
        ui->chkEnablePrinting->setChecked(Qt::Checked);
        on_chkEnablePrinting_stateChanged(Qt::Checked);
    } else {
        ui->chkEnablePrinting->setChecked(Qt::Unchecked);
        on_chkEnablePrinting_stateChanged(Qt::Unchecked);
    }

    if(pbs.getBool("printer", "confirmation"))
        ui->chkConfirmPrinting->setChecked(Qt::Checked);
    else
        ui->chkConfirmPrinting->setChecked(Qt::Unchecked);

    ui->spinPaperWidth->setValue(pbs.getInt("printer", "width"));
    ui->spinPaperHeight->setValue(pbs.getInt("printer", "height"));
    ui->spinMaxCopies->setValue(pbs.getInt("printer", "max_copies"));
    QString printBackend = pbs.get("printer", "backend");
    ui->cmbPrinterBackend->setCurrentText(mPrinterBackendMapping.value(printBackend));
    ui->editPrinterIP->setText(pbs.get("printer", "ip"));

    if(pbs.getBool("show", "enable")) {
        ui->chkEnableshow->setChecked(Qt::Checked);
        on_chkEnableshow_stateChanged(Qt::Checked);
    } else {
        ui->chkEnableshow->setChecked(Qt::Unchecked);
        on_chkEnableshow_stateChanged(Qt::Unchecked);
    }

    ui->spinShowWidth->setValue(pbs.getInt("show", "width"));
    ui->spinShowHeight->setValue(pbs.getInt("show", "height"));

    if(pbs.getBool("show", "fullscreen")) {
        ui->chkShowFullscreen->setChecked(Qt::Checked);
        on_chkShowFullscreen_stateChanged(Qt::Checked);
    } else {
        ui->chkShowFullscreen->setChecked(Qt::Unchecked);
        on_chkShowFullscreen_stateChanged(Qt::Unchecked);
    }

    ui->editShowFooterText->setText(pbs.get("show", "footertext"));
    ui->spinShowDisplayTime->setValue(pbs.getInt("show", "display_time"));
    if(pbs.getBool("show", "swap_screens"))
        ui->chkShowSwapScreens->setChecked(Qt::Checked);
    else
        ui->chkShowSwapScreens->setChecked(Qt::Unchecked);

    if(pbs.getBool("show", "full_images"))
        ui->chkShowFullImages->setChecked(Qt::Checked);
    else
        ui->chkShowFullImages->setChecked(Qt::Unchecked);

    if(pbs.getBool("show", "random"))
        ui->chkShowRandom->setChecked(Qt::Checked);
    else
        ui->chkShowRandom->setChecked(Qt::Unchecked);

    ui->editBaseDir->setText(pbs.get("storage", "basedir"));
    ui->editBasename->setText(pbs.get("storage", "basename"));
    ui->editIgnoreRemovable->setText(pbs.get("storage", "ignore_removable"));
    if(pbs.getBool("storage", "keep_pictures"))
        ui->chkKeep->setChecked(Qt::Checked);
    else
        ui->chkKeep->setChecked(Qt::Unchecked);

    if(pbs.getBool("storage", "wait_removable")) {
        ui->chkWaitRemovable->setChecked(Qt::Checked);
        on_chkWaitRemovable_stateChanged(Qt::Checked);
    } else {
        ui->chkWaitRemovable->setChecked(Qt::Unchecked);
        on_chkWaitRemovable_stateChanged(Qt::Unchecked);
    }
}

void settingsWidget::saveToSettings()
{
    pbSettings &pbs = pbSettings::getInstance();


    pbs.setBool("gui", "hide_cursor", ui->chkHideCursor->isChecked());
    pbs.setBool("gui", "direct_start", ui->chkStartDirectly->isChecked());
    pbs.setBool("gui", "fullscreen", ui->chkStartFullscreen->isChecked());

    pbs.setInt("gui", "height", ui->spinHeight->value());
    pbs.setInt("gui", "width", ui->spinWidth->value());

    pbs.set("gui", "style", mStyleMapping.key(ui->cmbStyle->currentText()));
    pbs.set("gui", "language", mLanguageMapping.key(ui->cmbLanguage->currentText()));

    pbs.setBool("archive", "enable", ui->chkEnableArchive->isChecked());
    pbs.setBool("archive", "allow_reprint", ui->chkAllowReprint->isChecked());

    pbs.set("camera", "backend", mCameraBackendMapping.key(ui->cmbBackend->currentText()));

    pbs.setInt("camera", "fps", ui->spinFps->value());
    pbs.setInt("camera", "rotation", mRotationMapping.key(ui->cmbRotation->currentText()));

    pbs.setBool("gpio", "enable", ui->chkEnableGPIO->isChecked());

    pbs.setInt("gpio", "af_lamp_pin", ui->spinAfLamp->value());
    pbs.setInt("gpio", "af_lamp_pwm_value", ui->spinAfValue->value());
    pbs.setInt("gpio", "idle_lamp_pin", ui->spinIdleLamp->value());
    pbs.setInt("gpio", "idle_lamp_pwm_value", ui->spinIdleValue->value());
    pbs.setInt("gpio", "trigger_pin", ui->spinTrigger->value());
    pbs.setInt("gpio", "exit_pin", ui->spinExit->value());

    pbs.setInt("qtphotobox", "countdown_time", ui->spinCountdown->value());
    pbs.setInt("qtphotobox", "display_time", ui->spinDisplay->value());
    pbs.setInt("qtphotobox", "greeter_time", ui->spinGreeter->value());
    pbs.setInt("qtphotobox", "postprocess_time", ui->spinPostprocessing->value());
    pbs.setBool("qtphotobox", "show_preview", ui->chkPreview->isChecked());

    pbs.set("picture", "background", ui->editBackground->text());
    pbs.setInt("picture", "num_pictures", ui->spinNumPictures->value());
    pbs.setInt("picture", "size_x", ui->spinPictureX->value());
    pbs.setInt("picture", "size_y", ui->spinPictureY->value());

    for(int i=0; i<ui->spinNumPictures->value(); i++) {
        pbs.set("picture", "filters_picture" + QString::number(i+1), mFilterEdits.at(i)->text());
    }

    pbs.setBool("printer", "enable", ui->chkEnablePrinting->isChecked());
    pbs.setBool("printer", "confirmation", ui->chkConfirmPrinting->isChecked());
    pbs.setInt("printer", "width", ui->spinPaperWidth->value());
    pbs.setInt("printer", "height", ui->spinPaperHeight->value());
    pbs.setInt("printer", "max_copies", ui->spinMaxCopies->value());
    pbs.set("printer", "backend", mPrinterBackendMapping.key(ui->cmbPrinterBackend->currentText()));
    pbs.set("printer", "ip", ui->editPrinterIP->text());

    pbs.setBool("show", "enable", ui->chkEnableshow->isChecked());
    pbs.setInt("show", "width", ui->spinShowWidth->value());
    pbs.setInt("show", "height", ui->spinShowHeight->value());
    pbs.setBool("show", "fullscreen", ui->chkShowFullscreen->isChecked());
    pbs.set("show", "footertext", ui->editShowFooterText->text());
    pbs.setInt("show", "display_time", ui->spinShowDisplayTime->value());
    pbs.setBool("show", "swap_screens", ui->chkShowSwapScreens->isChecked());
    pbs.setBool("show", "full_images", ui->chkShowFullImages->isChecked());
    pbs.setBool("show", "random", ui->chkShowRandom->isChecked());

    pbs.set("storage", "basedir", ui->editBaseDir->text());
    pbs.set("storage", "basename", ui->editBasename->text());
    pbs.set("storage", "ignore_removable", ui->editIgnoreRemovable->text());
    pbs.setBool("storage", "keep_pictures", ui->chkKeep->isChecked());
    pbs.setBool("storage", "wait_removable", ui->chkWaitRemovable->isChecked());

    QString path = pbs.getConfigPath();
    path += "settings.ini";
    pbs.saveConfigFile(path);
}

void settingsWidget::on_btnSave_clicked()
{
    saveToSettings();
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

void settingsWidget::on_chkEnableshow_stateChanged(int arg1)
{
    bool enabled = true;
    if(arg1 == Qt::Checked) {
        enabled = true;
    } else {
        enabled = false;
    }
    ui->chkShowFullImages->setEnabled(enabled);
    ui->chkShowFullscreen->setEnabled(enabled);
    ui->chkShowRandom->setEnabled(enabled);
    ui->chkShowSwapScreens->setEnabled(enabled);
    ui->editShowFooterText->setEnabled(enabled);
    ui->spinShowDisplayTime->setEnabled(enabled);
    ui->spinShowWidth->setEnabled(enabled);
    ui->spinShowHeight->setEnabled(enabled);
}

void settingsWidget::on_chkShowFullscreen_stateChanged(int arg1)
{
    bool enabled;
    if(arg1 == Qt::Checked) {
        enabled = false;
    } else {
        enabled = true;
    }
    ui->spinShowWidth->setEnabled(enabled);
    ui->spinShowHeight->setEnabled(enabled);
}

void settingsWidget::on_chkEnableArchive_stateChanged(int arg1)
{
    bool enabled;
    if(arg1 == Qt::Checked) {
        enabled = true;
    } else {
        enabled = false;
    }
    ui->chkAllowReprint->setEnabled(enabled);
}

void settingsWidget::on_chkEnableGPIO_stateChanged(int arg1)
{
    bool enabled;
    if(arg1 == Qt::Checked) {
        enabled = true;
    } else {
        enabled = false;
    }
    ui->spinAfLamp->setEnabled(enabled);
    ui->spinAfValue->setEnabled(enabled);
    ui->spinExit->setEnabled(enabled);
    ui->spinIdleLamp->setEnabled(enabled);
    ui->spinIdleValue->setEnabled(enabled);
    ui->spinTrigger->setEnabled(enabled);
}

void settingsWidget::on_chkWaitRemovable_stateChanged(int arg1)
{
    bool enabled;
    if(arg1 == Qt::Checked) {
        enabled = true;
    } else {
        enabled = false;
    }
    ui->editIgnoreRemovable->setEnabled(enabled);
}

void settingsWidget::on_chkEnablePrinting_stateChanged(int arg1)
{
    bool enabled;
    if(arg1 == Qt::Checked) {
        enabled = true;
    } else {
        enabled = false;
    }
    ui->chkConfirmPrinting->setEnabled(enabled);
    ui->cmbPrinterBackend->setEnabled(enabled);
    ui->editPrinterIP->setEnabled(enabled);
    ui->spinMaxCopies->setEnabled(enabled);
    ui->spinPaperWidth->setEnabled(enabled);
    ui->spinPaperHeight->setEnabled(enabled);
}

void settingsWidget::on_btnBackground_clicked()
{
    QString dir;
    QString existing = ui->editBackground->text();
    if(!existing.isNull()) {
        QFileInfo file(existing);
        if(file.exists()) {
            dir = file.filePath();
        }
    }

    QString filename = QFileDialog::getOpenFileName(this, tr("Open Background image"),
                                                    dir, tr("JPEG Images (*.jpg)"));
    if(!filename.isEmpty()) {
        ui->editBackground->setText(filename);
    }
}

void settingsWidget::on_chkStartFullscreen_stateChanged(int arg1)
{
    bool enabled;
    if(arg1 == Qt::Checked) {
        enabled = false;
    } else {
        enabled = true;
    }
    ui->spinWidth->setEnabled(enabled);
    ui->spinHeight->setEnabled(enabled);
}

void settingsWidget::on_spinNumPictures_valueChanged(int arg1)
{
    pbSettings &pbs = pbSettings::getInstance();

    QGridLayout *lay = qobject_cast<QGridLayout*>(ui->tabPicture->layout());
    for(int i=0; i<lay->count(); i++) {
        QLayoutItem *item = lay->itemAt(i);
        if(item->spacerItem()) {
            lay->removeItem(item);
            delete item;
            i--;
        }
    }

    for(int i=0; i<mFilterEdits.size(); i++) {
        mFilterEdits.at(i)->setVisible(false);
        mFilterLabels.at(i)->setVisible(false);
    }

    for(int i=0; i<arg1; i++) {
        QString editName = QString("editFilter%1").arg(i+1);
        QString lblName = QString("lblFilter%1").arg(i+1);

        if(i >= mFilterEdits.size()) {
            QLabel *lbl = new QLabel(tr("Filters for Picture %1").arg(i+1), this);
            QLineEdit *edit = new QLineEdit(this);
            QString text = pbs.get("picture", "filters_picture" + QString::number(i+1));

            lbl->setObjectName(lblName);
            edit->setText(text);
            edit->setObjectName(editName);
            mFilterEdits.append(edit);
            mFilterLabels.append(lbl);

            lbl->setVisible(true);
            edit->setVisible(true);

            int row = lay->rowCount();
            lay->addWidget(edit, row, 1);
            lay->addWidget(lbl, row, 2);
        } else {
            mFilterEdits.at(i)->setVisible(true);
            mFilterLabels.at(i)->setVisible(true);
        }
    }
    QSpacerItem *newSpacer = new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QSpacerItem *newSpacer2 = new QSpacerItem(1, 1, QSizePolicy::Expanding, QSizePolicy::Minimum);
    lay->addItem(newSpacer, lay->rowCount(), 1);
    lay->addItem(newSpacer2, 1, lay->columnCount());
}
