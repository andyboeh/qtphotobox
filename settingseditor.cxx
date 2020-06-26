#include "settingseditor.h"
#include "settingswidget.h"
#include <QApplication>

MainWindow::MainWindow()
{
    settingsWidget *widget = new settingsWidget();
    setCentralWidget(widget);
    show();
}

MainWindow::~MainWindow() {

}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;

    app.exec();
}
