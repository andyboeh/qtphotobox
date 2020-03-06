#ifndef _PICTURETASK_H
#define _PICTURETASK_H

#include <QPixmap>
#include <QList>

class pictureTask {
public:
    typedef enum {
        TASK_TYPE_NONE = 0,
        TASK_TYPE_ASSEMBLE_PICTURE,
        TASK_TYPE_SCALE_THUMBNAIL,
    } eTaskType;

    pictureTask();
    pictureTask(eTaskType type);
    void setTaskType(eTaskType type);
    eTaskType getTaskType();
    ~pictureTask();
    void addPicture(QPixmap image);
    int getNumPictures(void);
    QList<QPixmap> getImages();
private:
    eTaskType mTaskType;
    QList<QPixmap> mImages;
};

#endif //_PICTURETASK_H
