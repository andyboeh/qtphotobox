#ifndef _PICTURETASK_H
#define _PICTURETASK_H

#include <QPixmap>
#include <QList>

typedef enum {
    TASK_TYPE_NONE = 0,
    TASK_TYPE_ASSEMBLE_PICTURE,
} eTaskType;

class pictureTask {
public:
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
