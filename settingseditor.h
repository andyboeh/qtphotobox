#ifndef _SETTINGSEDITOR_H
#define _SETTINGSEDITOR_H

#include <QMainWindow>

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow();
    ~MainWindow();
private:
    void askLoadFile();
};

#endif //_SETTINGSEDITOR_H
