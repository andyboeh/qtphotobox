#include "postprocessTask.h"


postprocessTask::postprocessTask()
{

}

postprocessTask::postprocessTask(ePostprocessType type)
{
    mTaskType = type;
}

void postprocessTask::setTaskType(ePostprocessType type)
{
    mTaskType = type;
}

ePostprocessType postprocessTask::getTaskType()
{
    return mTaskType;
}

postprocessTask::~postprocessTask()
{

}

void postprocessTask::setPicture(QPixmap image)
{
    mImage = image;
}

QPixmap postprocessTask::getImage()
{
    return mImage;
}
