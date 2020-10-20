#include "settingseditor.h"
#include "settingswidget.h"
#include "settings.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>

MainWindow::MainWindow()
{
    settingsWidget *widget = new settingsWidget();
    setCentralWidget(widget);
    show();
    askLoadFile();
}

void MainWindow::askLoadFile() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Load File?"), tr("Do you want to load a settings file to start with?"),
                                  QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes) {
        QString filename = QFileDialog::getOpenFileName(this, tr("Open Settings File"),
                                                        "", tr("Settings File (*.ini)"));
        if(!filename.isEmpty()) {
            pbSettings &pbs = pbSettings::getInstance();
            pbs.mergeConfigFile(filename);
            settingsWidget *widget = dynamic_cast<settingsWidget*>(centralWidget());
            widget->loadFromSettings();
        }
    }
}

MainWindow::~MainWindow() {

}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;

    app.exec();
}
