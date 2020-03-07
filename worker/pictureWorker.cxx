#include "pictureWorker.h"
#include "settings.h"
#include <QPainter>

pictureWorker::pictureWorker()
{
    mTaskOpened = false;
}

pictureWorker::~pictureWorker()
{

}

void pictureWorker::initAssembleTask()
{
    initTask(pictureTask::TASK_TYPE_ASSEMBLE_PICTURE);
}

void pictureWorker::cancelTask()
{
    mTaskOpened = false;
    mOpenPictureTask = pictureTask();
}

void pictureWorker::addPicture(QPixmap image)
{
    if(!mTaskOpened)
        return;
    mOpenPictureTask.addPicture(image);
}

void pictureWorker::finishTask()
{
    if(!mTaskOpened)
        return;
    mPictureTasks.append(mOpenPictureTask);
    mOpenPictureTask = pictureTask();
    mCommandList.append("processTask");
}

void pictureWorker::start()
{
    qDebug() << QThread::currentThreadId();
    qDebug() << "Thread start.";

    bool running = true;

    mFilterList.clear();
    pbSettings &pbs = pbSettings::getInstance();
    int numpics = pbs.getInt("picture", "num_pictures");
    for(int i=0; i<numpics; i++) {
        QString filter = pbs.get("picture", "filters_picture" + QString::number(i+1));
        mFilterList.append(filter);
    }

    mPictureSizeX = pbs.getInt("picture", "size_x");
    mPictureSizeY = pbs.getInt("picture", "size_y");

    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            continue;

        QString command = mCommandList.takeFirst();
        if(command == "processTask") {
            if(mPictureTasks.isEmpty())
                continue;
            pictureTask task = mPictureTasks.takeFirst();
            switch(task.getTaskType()) {
            case pictureTask::TASK_TYPE_ASSEMBLE_PICTURE:
            {
                qDebug() << "TASK_TYPE_ASSEMBLE_PICTURE";
                QPixmap image = assemblePictureTask(task);
                if(image.isNull())
                    emit pictureError(tr("Error assembling picture."));
                else
                    emit pictureAssembled(image);
                break;
            }
            default:
                break;
            }
        } else if(command == "stopThread") {
            running = false;
        }
    }

    emit finished();
}

void pictureWorker::stop()
{
    qDebug() << "stopThread";
    mCommandList.append("stopThread");
}

void pictureWorker::initTask(pictureTask::eTaskType type)
{
    mTaskOpened = true;
    mOpenPictureTask = pictureTask(type);
}

QPixmap pictureWorker::assemblePictureTask(pictureTask task)
{
    QList<QPixmap> images = task.getImages();
    int numPics;
    if(images.size() != mFilterList.size()) {
        qDebug() << "Number of images does not match number of filters";
        numPics = qMin<int>(images.size(), mFilterList.size());
    } else {
        numPics = images.size();
    }

    qDebug() << "Assembling " << numPics << "pictures.";

    QPixmap image(mPictureSizeX, mPictureSizeY);
    qDebug() << "New image dimensions: " << image.width() << " x " << image.height();
    QPainter painter(&image);
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(image.rect(), Qt::white);

    for(int i=0; i<numPics; i++) {
        int dx = 0;
        int dy = 0;
        QString filter = mFilterList.at(i);
        QPixmap imageToDraw = images.at(i);
        if(!filter.isEmpty()) {
            QStringList filters = filter.split(",");
            foreach(QString filter, filters) {
                QStringList filterArgs = filter.split(":");
                QString op = filterArgs.at(0);
                if(op == "scale") {
                    int fr = filterArgs.at(1).toInt();
                    int nw = image.width() * fr / 100;
                    qDebug() << "Applying scaling: " << fr << " width: " << nw;
                    imageToDraw = imageToDraw.scaledToWidth(nw, Qt::SmoothTransformation);
                } else if(op == "move") {
                    qDebug() << "Applying move";
                    for(int i=1; i<filterArgs.size(); i++) {
                        QStringList opArgs = filterArgs.at(i).split("=");
                        int off = opArgs.at(1).toInt();
                        if(opArgs.at(0) == "top") {
                            dy = off;
                        } else if(opArgs.at(0) == "bottom") {
                            dy = image.height() - off - imageToDraw.height();
                        } else if(opArgs.at(0) == "left") {
                            dx = off;
                        } else if(opArgs.at(0) == "right") {
                            dx = image.width() - off - imageToDraw.width();
                        }
                    }
                }
            }
            if(dx < 0)
                dx = 0;
            if(dy < 0)
                dy = 0;
            painter.setCompositionMode(QPainter::CompositionMode_Source);
            painter.drawImage(dx, dy, imageToDraw.toImage());
        } else {
            qDebug() << "Empty filter for picture " << i;
        }
    }
    painter.end();
    return image;
}
