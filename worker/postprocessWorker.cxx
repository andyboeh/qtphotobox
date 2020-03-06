#include "postprocessWorker.h"
#include "settings.h"
#include "storageManager.h"
#include <QDir>
#include <QDebug>

postprocessWorker::postprocessWorker()
{

}

postprocessWorker::~postprocessWorker()
{

}

void postprocessWorker::saveFullImage(QPixmap image)
{
    postprocessTask task(POSTPROCESS_TYPE_SAVE_FULL_IMAGE);
    task.setPicture(image);
    mPostprocessTasks.append(task);
    mCommandList.append("processTask");
}

void postprocessWorker::saveAssembledImage(QPixmap image)
{
    postprocessTask task(POSTPROCESS_TYPE_SAVE_ASSEMBLED_IMAGE);
    task.setPicture(image);
    mPostprocessTasks.append(task);
    mCommandList.append("processTask");
}

void postprocessWorker::start()
{
    qDebug() << QThread::currentThreadId();
    qDebug() << "Thread start.";

    bool running = true;
    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            continue;

        QString command = mCommandList.takeFirst();

        if(command == "processTask") {
            qDebug() << "processTask";
            if(mPostprocessTasks.isEmpty())
                continue;
            postprocessTask task = mPostprocessTasks.takeFirst();

            switch(task.getTaskType()) {
            case POSTPROCESS_TYPE_SAVE_FULL_IMAGE:
                qDebug() << "saveFullImage";
                saveFullImageReal(task.getImage());
                break;
            case POSTPROCESS_TYPE_SAVE_ASSEMBLED_IMAGE:
                qDebug() << "saveAssembledImage";
                saveAssembledImageReal(task.getImage());
                break;
            default:
                break;
            }

        } else if(command == "stopThread") {
            running = false;
        }
    }

    emit finished();
}

void postprocessWorker::stop()
{
    mCommandList.append("stopThread");
}

bool postprocessWorker::saveFullImageReal(QPixmap image)
{
    storageManager &sm = storageManager::getInstance();
    QString path = sm.getPictureStoragePath();
    QString filename = sm.getNextFilename(path, storageManager::FILETYPE_FULL);
    return image.save(path + QDir::separator() + filename, "JPG");
}

bool postprocessWorker::saveAssembledImageReal(QPixmap image)
{
    storageManager &sm = storageManager::getInstance();
    QString path = sm.getPictureStoragePath();
    QString filename = sm.getNextFilename(path, storageManager::FILETYPE_ASSEMBLED);
    return image.save(path + QDir::separator() + filename, "JPG");
}
