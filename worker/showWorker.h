#ifndef _SHOWWORKER_H
#define _SHOWWORKER_H

#include "worker.h"
#include <QStringList>
#include "pictureTask.h"

class showWidget;
class QTimer;

class showWorker : public Worker
{
    Q_OBJECT
public:
    showWorker();
    ~showWorker();
public slots:
    void showRandomPicture();
    void showNextPicture();
    void scanPictures();
    void addPicture(QString path);
    void start();
    void stop();
signals:
    void showPicture(QPixmap image);
private:
    QStringList mFileList;
    bool mTaskOpened;
    showWidget *mWidget;
    QTimer *mTimer;
    int mCurrentIndex;
};

#endif //_SHOWWORKER_H
