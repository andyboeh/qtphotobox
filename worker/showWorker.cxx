#include "showWorker.h"
#include "showwidget.h"
#include "settings.h"
#include "storageManager.h"
#include <QTimer>
#include <QPixmap>
#include <QDir>
#include <QRandomGenerator>
#include <QWindow>
#include <QApplication>
#include <QScreen>

showWorker::showWorker()
{
    mWidget = nullptr;
}

showWorker::~showWorker()
{
    delete mWidget;
    mWidget = nullptr;
}

void showWorker::showRandomPicture()
{
    mCommandList.append("showRandomPicture");
}

void showWorker::showNextPicture()
{
    mCommandList.append("showNextPicture");
}

void showWorker::scanPictures()
{
    mCommandList.append("scanPictures");
}

void showWorker::addPicture(QString path)
{
    mFileList.append(path);
}

void showWorker::start()
{
    bool running = true;

    pbSettings &pbs = pbSettings::getInstance();
    int displaytime = pbs.getInt("show", "display_time");
    pbs.getBool("show", "swap_screens");
    int width = pbs.getInt("show", "width");
    int height = pbs.getInt("show", "height");
    QString footertext = pbs.get("show", "footertext");

    mWidget = new showWidget(footertext);

    if(pbs.getBool("show", "fullscreen")) {
        if(pbs.getBool("show", "swap_screens")) {
            mWidget->setGeometry(QGuiApplication::screens().first()->geometry());
        } else {
            mWidget->setGeometry(QGuiApplication::screens().last()->geometry());
        }
        mWidget->showFullScreen();
    } else {
        mWidget->setFixedSize(width, height);
        mWidget->show();
    }

    connect(this, SIGNAL(showPicture(QPixmap)), mWidget, SLOT(showPicture(QPixmap)));

    storageManager &sm = storageManager::getInstance();
    QString basedir = sm.getPictureStoragePath();
    QString basename = pbs.get("storage", "basename");
    QString filter;
    if(pbs.getBool("show", "full_images")) {
        filter = basename + "-full-*";
    } else {
        filter = basename + "-assembled-*";
    }
    QStringList filterlist;
    filterlist.append(filter);

    QDir dir(basedir);
    mFileList = dir.entryList(filterlist);

    mTimer = new QTimer();
    mTimer->setSingleShot(false);
    mTimer->setInterval(displaytime * 1000);
    if(pbs.getBool("show", "random"))
        connect(mTimer, SIGNAL(timeout()), this, SLOT(showRandomPicture()));
    else
        connect(mTimer, SIGNAL(timeout()), this, SLOT(showNextPicture()));
    mTimer->start();

    mCurrentIndex = 0;

    emit started();

    while(running) {
        waitForCommand();
        if(mCommandList.isEmpty())
            continue;

        QString command = mCommandList.takeFirst();

        if(command == "showRandomPicture") {
            if(mFileList.isEmpty()) {
                scanPictures();
                continue;
            }
            QRandomGenerator rand(0, mFileList.size()-1);
            QString file = basedir + QDir::separator() + mFileList.takeAt(rand.generate());
            QPixmap image(file);
            emit showPicture(image);
        } else if(command == "showNextPicture") {
            if(mFileList.isEmpty())
                continue;
            QString file = basedir + QDir::separator() + mFileList.at(mCurrentIndex);
            QPixmap image(file);
            mCurrentIndex++;
            if(mCurrentIndex == mFileList.size())
                mCurrentIndex = 0;
            emit showPicture(image);
        } else if(command == "scanPictures") {
            mFileList = dir.entryList(filterlist);
        }
        else if(command == "stopThread") {
            running = false;
        }
    }

    mTimer->stop();
    delete mTimer;
    mTimer = nullptr;
    delete mWidget;
    mWidget = nullptr;
    emit finished();
}

void showWorker::stop()
{
    mCommandList.append("stopThread");
}
