#ifndef _PICTUREWORKER_H
#define _PICTUREWORKER_H


#include "worker.h"
#include <QPixmap>
#include <QList>
#include <QStringList>
#include "pictureTask.h"

class pictureWorker : public Worker
{
    Q_OBJECT
public:
    pictureWorker();
    ~pictureWorker();
public slots:
    void initAssembleTask();
    void cancelTask();
    void addPicture(QPixmap image);
    void finishTask();
    void start();
    void stop();
signals:
    void pictureAssembled(QPixmap image);
private:
    QStringList mFilterList;
    bool mTaskOpened;
    pictureTask mOpenPictureTask;
    void initTask(pictureTask::eTaskType type);
    QPixmap assemblePictureTask(pictureTask task);
    QList<pictureTask> mPictureTasks;
    int mPictureSizeX;
    int mPictureSizeY;
};


#endif //_PICTUREWORKER_H
