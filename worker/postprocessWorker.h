#ifndef _POSTPROCESSWORKER_H
#define _POSTPROCESSWORKER_H

#include "worker.h"
#include <QPixmap>
#include <QList>
#include "postprocessTask.h"

class postprocessWorker : public Worker
{
    Q_OBJECT
public:
    postprocessWorker();
    ~postprocessWorker();
public slots:
    void saveThumbnail(QString file);
    void saveFullImage(QPixmap image);
    void saveAssembledImage(QPixmap image);
    void start();
    void stop();
signals:
    void fullImageSaved(QString path, bool ret);
    void assembledImageSaved(QString path, bool ret);
    void thumbnailScaled(QString path);
private:
    QList<postprocessTask> mPostprocessTasks;
    bool saveFullImageReal(QPixmap image);
    bool saveAssembledImageReal(QPixmap image);
    bool saveThumbnailReal(QString filename);
};


#endif //_POSTPROCESSWORKER_H
