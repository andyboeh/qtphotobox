#include "pictureWorker.h"
#include "settings.h"
#include <QFile>
#include "storageManager.h"
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


    QString backgroundImage = pbs.get("picture", "background");
    if(!backgroundImage.isEmpty()) {
        QFile file(backgroundImage);
        if(!file.exists()) {
            emit pictureError(tr("Background image not found: %1").arg(backgroundImage));
        } else {
            mBackgroundImage = QPixmap(backgroundImage).scaled(mPictureSizeX, mPictureSizeY, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }

    emit started();

    while(running) {
        waitForCommand();

        if(mCommandList.isEmpty())
            continue;

        mMutex.lock();
        QString command = mCommandList.takeFirst();
        mMutex.unlock();
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
    mMutex.lock();
    mCommandList.append("stopThread");
    mMutex.unlock();
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
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if(mBackgroundImage.isNull()) {
        painter.fillRect(image.rect(), Qt::white);
    } else {
        painter.drawImage(0, 0, mBackgroundImage.toImage());
    }

    for(int i=0; i<numPics; i++) {
        int dx = 0;
        int dy = 0;
        QString filter = mFilterList.at(i);
        QPixmap imageToDraw(images.at(i));

        if(!filter.isEmpty()) {
            QStringList filters = filter.split(",");
            foreach(QString filter, filters) {
                QStringList filterArgs = filter.split(":");
                QString op = filterArgs.at(0);
                if(op == "scale") {
                    bool sw = false;
                    bool sh = false;
                    int fw = 100;
                    int fh = 100;
                    int nw;
                    int nh;
                    for(int i=1; i<filterArgs.size(); i++) {
                        QStringList opArgs = filterArgs.at(i).split("=");
                        QString scale = opArgs.at(1);
                        if(opArgs.at(0) == "width") {
                            sw = true;
                            if(scale.contains("%")) {
                                fw = scale.remove("%").trimmed().toInt();
                                nw = image.width() * fw / 100;
                            } else {
                                nw = scale.toInt();
                            }
                        } else if(opArgs.at(0) == "height") {
                            sh = true;
                            if(scale.contains("%")) {
                                fh = scale.remove("%").trimmed().toInt();
                                nh = image.height() * fh / 100;
                            } else {
                                nh = scale.toInt();
                            }
                        }
                    }
                    if(sw && sh) {
                        qDebug() << "Applying width and height scaling, width = " << nw << ", height = " << nh;
                        imageToDraw = imageToDraw.scaled(nw, nh, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
                    } else if(sw) {
                        qDebug() << "Applying width scaling, width = " << nw;
                        imageToDraw = imageToDraw.scaledToWidth(nw, Qt::SmoothTransformation);
                    } else if(sh) {
                        qDebug() << "Applying height scaling, height = " << nh;
                        imageToDraw = imageToDraw.scaledToHeight(nh, Qt::SmoothTransformation);
                    }
                } else if(op == "grayscale") {
                    qDebug() << "Applying grayscale";
                    imageToDraw = QPixmap::fromImage(imageToDraw.toImage().convertToFormat(QImage::Format_Grayscale8));
                } else if(op == "copy") {
                    qDebug() << "Applying copy";
                    // For copy, we simply apply all operations so far
                    // and restart with the same image again.
                    if(dx < 0)
                        dx = 0;
                    if(dy < 0)
                        dy = 0;
                    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
                    painter.drawPixmap(dx, dy, imageToDraw);
                    dx = 0;
                    dy = 0;
                    imageToDraw = images.at(i);
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
                } else if(op == "rotate") {
                    int angle = filterArgs.at(1).toInt();
                    qDebug() << "Applying rotation: " << angle;
                    QTransform transform = QTransform().rotate(angle);
                    imageToDraw = imageToDraw.transformed(transform);
                } else if(op == "crop") {
                    qDebug() << "Applying crop";
                    int x = 0;
                    int y = 0;
                    int w = imageToDraw.width();
                    int h = imageToDraw.height();
                    for(int i=1; i<filterArgs.size(); i++) {
                        QStringList opArgs = filterArgs.at(i).split("=");
                        int param = opArgs.at(1).toInt();
                        if(opArgs.at(0) == "x") {
                            x = param;
                        } else if(opArgs.at(0) == "y") {
                            y = param;
                        } else if(opArgs.at(0) == "w") {
                            w = param;
                        } else if(opArgs.at(0) == "h") {
                            h = param;
                        }
                    }
                    if(x > imageToDraw.width())
                        x = imageToDraw.width();
                    if(y > imageToDraw.height())
                        y = imageToDraw.height();
                    if(w > x + imageToDraw.width())
                        w = imageToDraw.width() - x;
                    if(h > y + imageToDraw.height())
                        h = imageToDraw.height() - y;
                    imageToDraw = imageToDraw.copy(x, y, w, h);
                } else if(op == "border") {
                    qDebug() << "Applying border";
                    int size = 5;
                    QColor color(0,0,0,180);
                    for(int i=1; i<filterArgs.size(); i++) {
                        QStringList opArgs = filterArgs.at(i).split("=");
                        if(opArgs.at(0) == "size") {
                            size = opArgs.at(1).toInt();
                        } else if(opArgs.at(0) == "style") {
                            QString style = opArgs.at(1);
                            if(style == "black")
                                color = QColor(0,0,0,180);
                            else if(style == "white")
                                color = QColor(255,255,255,180);
                        }
                    }
                    QPixmap dst(imageToDraw.width() + size * 2, imageToDraw.height() + size * 2);
                    dst.fill(color);
                    QPainter pnt(&dst);
                    pnt.setCompositionMode(QPainter::CompositionMode_SourceOver);
                    pnt.drawPixmap(size, size, imageToDraw);
                    pnt.end();
                    imageToDraw = dst;
                }
            }
            if(dx < 0)
                dx = 0;
            if(dy < 0)
                dy = 0;
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.drawPixmap(dx, dy, imageToDraw);
        } else {
            qDebug() << "Empty filter for picture " << i;
        }
    }
    painter.end();
    return image;
}
