#include "pictureTask.h"

pictureTask::pictureTask()
{
    mTaskType = TASK_TYPE_NONE;
}

pictureTask::pictureTask(eTaskType type)
{
    mTaskType = type;
}

void pictureTask::setTaskType(eTaskType type)
{
    mTaskType = type;
}

eTaskType pictureTask::getTaskType()
{
    return mTaskType;
}

pictureTask::~pictureTask()
{

}

void pictureTask::addPicture(QPixmap image)
{
    mImages.append(image);
}

int pictureTask::getNumPictures()
{
    return mImages.size();
}

QList<QPixmap> pictureTask::getImages()
{
    return mImages;
}
