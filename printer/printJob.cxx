#include "printJob.h"
#include <QTemporaryFile>

printJob::printJob()
{
    mCopies = 0;
    mCopiesPrinted = 0;
    mWidth = 0;
    mHeight = 0;
    mSpoolFile = "";
}

printJob::printJob(QPixmap image)
{
    mWidth = 0;
    mHeight = 0;
    mCopies = 1;
    mCopiesPrinted = 0;
    mImage = image;
    mSpoolFile = "";
}

printJob::printJob(QPixmap image, int copies)
{
    mWidth = 0;
    mHeight = 0;
    mImage = image;
    mCopies = copies;
    mCopiesPrinted = 0;
    mSpoolFile = "";
}

printJob::printJob(QString filename, int copies)
{
    mWidth = 0;
    mHeight = 0;
    mFilename = filename;
    mCopies = copies;
    mCopiesPrinted = 0;
    mSpoolFile = "";
}

printJob::~printJob()
{
    if(mRemoveFile) {
        QFile file(mFilename);
        if(file.exists())
            file.remove();
    }
    if(!mSpoolFile.isEmpty()) {
        QFile file(mSpoolFile);
        if(file.exists())
            file.remove();
    }
}

bool printJob::isFileJob()
{
    return !mFilename.isEmpty();
}

void printJob::setCopies(int copies)
{
    mCopies = copies;
}

int printJob::getCopies()
{
    return mCopies;
}

int printJob::getCopiesPrinted()
{
    return mCopiesPrinted;
}

void printJob::setCopiesPrinted(int copies)
{
    mCopiesPrinted = copies;
}

QPixmap printJob::getImage() {
    return mImage;
}

QString printJob::getFile()
{
    return mFilename;
}

QString printJob::getSpoolFile()
{
    if(!QFile(mSpoolFile).exists()) {
        QTemporaryFile file;
        file.open();
        file.setAutoRemove(false);
        mSpoolFile = file.fileName();
        file.close();
    }
    return mSpoolFile;
}

int printJob::getWidth()
{
    if(mWidth > 0)
        return mWidth;

    if(mFilename.isEmpty()) {
        mWidth = mImage.width();
        return mWidth;
    } else {
        mWidth = QPixmap(mFilename).width();
        return mWidth;
    }
}

int printJob::getHeight()
{
    if(mHeight > 0)
        return mHeight;

    if(mFilename.isEmpty()) {
        mHeight = mImage.height();
        return mHeight;
    } else {
        mHeight = QPixmap(mFilename).height();
        return mHeight;
    }
}

bool printJob::getRemoveFile()
{
    return mRemoveFile;
}

void printJob::setRemoveFile(bool remove)
{
    mRemoveFile = remove;
}
