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

void postprocessWorker::saveThumbnail(QString file)
{
    postprocessTask task(postprocessTask::POSTPROCESS_TYPE_CREATE_THUMBNAIL);
    task.setFile(file);
    mMutex.lock();
    mPostprocessTasks.append(task);
    mCommandList.append("processTask");
    mMutex.unlock();
}

void postprocessWorker::saveFullImage(QPixmap image)
{
    postprocessTask task(postprocessTask::POSTPROCESS_TYPE_SAVE_FULL_IMAGE);
    task.setPicture(image);
    mMutex.lock();
    mPostprocessTasks.append(task);
    mCommandList.append("processTask");
    mMutex.unlock();
}

void postprocessWorker::saveAssembledImage(QPixmap image)
{
    postprocessTask task(postprocessTask::POSTPROCESS_TYPE_SAVE_ASSEMBLED_IMAGE);
    task.setPicture(image);
    mMutex.lock();
    mPostprocessTasks.append(task);
    mCommandList.append("processTask");
    mMutex.unlock();
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

        mMutex.lock();
        QString command = mCommandList.takeFirst();
        mMutex.unlock();

        if(command == "processTask") {
            qDebug() << "processTask";
            if(mPostprocessTasks.isEmpty())
                continue;
            mMutex.lock();
            postprocessTask task = mPostprocessTasks.takeFirst();
            mMutex.unlock();

            switch(task.getTaskType()) {
            case postprocessTask::POSTPROCESS_TYPE_SAVE_FULL_IMAGE:
                qDebug() << "saveFullImage";
                if(!saveFullImageReal(task.getImage()))
                    emit postprocessError(tr("Error saving full image."));
                break;
            case postprocessTask::POSTPROCESS_TYPE_SAVE_ASSEMBLED_IMAGE:
                qDebug() << "saveAssembledImage";
                if(!saveAssembledImageReal(task.getImage()))
                    emit postprocessError(tr("Error saving assembled image."));
                break;
            case postprocessTask::POSTPROCESS_TYPE_CREATE_THUMBNAIL:
                qDebug() << "createThumbnail";
                if(!saveThumbnailReal(task.getFile()))
                    emit postprocessError(tr("Error saving thumbnail image."));
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
    mMutex.lock();
    mCommandList.append("stopThread");
    mMutex.unlock();
}

bool postprocessWorker::saveFullImageReal(QPixmap image)
{
    bool ret;
    storageManager &sm = storageManager::getInstance();
    QString path = sm.getPictureStoragePath();
    QString filename = sm.getNextFilename(path, storageManager::FILETYPE_FULL);
    ret = image.save(path + QDir::separator() + filename, "JPG");
    emit fullImageSaved(path, filename, ret);
    return ret;
}

bool postprocessWorker::saveAssembledImageReal(QPixmap image)
{
    bool ret;
    storageManager &sm = storageManager::getInstance();
    QString path = sm.getPictureStoragePath();
    QString filename = sm.getNextFilename(path, storageManager::FILETYPE_ASSEMBLED);
    ret = image.save(path + QDir::separator() + filename, "JPG");
    emit assembledImageSaved(path, filename, ret);
    return ret;
}

bool postprocessWorker::saveThumbnailReal(QString filename)
{
    bool ret;
    storageManager &sm = storageManager::getInstance();
    QString path = sm.getPictureStoragePath();
    QFile picture(path + QDir::separator() + filename);
    if(!picture.exists())
        return false;

    QPixmap image(path + QDir::separator() + filename);
    QString savePath = sm.getThumbnailStoragePath();
    QPixmap scaled = image.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ret = scaled.save(savePath + QDir::separator() + filename);
    emit thumbnailScaled(savePath, filename);
    return ret;
}
