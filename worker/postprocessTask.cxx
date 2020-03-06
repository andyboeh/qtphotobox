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

postprocessTask::ePostprocessType postprocessTask::getTaskType()
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

void postprocessTask::setFile(QString filename)
{
    mFilename = filename;
}

QPixmap postprocessTask::getImage()
{
    return mImage;
}

QString postprocessTask::getFile()
{
    return mFilename;
}
