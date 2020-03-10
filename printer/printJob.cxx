#include "printJob.h"

printJob::printJob()
{
    mCopies = 0;
    mCopiesPrinted = 0;
    mWidth = 0;
    mHeight = 0;
}

printJob::printJob(QPixmap image)
{
    mWidth = 0;
    mHeight = 0;
    mCopies = 1;
    mCopiesPrinted = 0;
    mImage = image;
}

printJob::printJob(QPixmap image, int copies)
{
    mWidth = 0;
    mHeight = 0;
    mImage = image;
    mCopies = copies;
    mCopiesPrinted = 0;

}

printJob::printJob(QString filename, int copies)
{
    mWidth = 0;
    mHeight = 0;
    mFilename = filename;
    mCopies = copies;
    mCopiesPrinted = 0;

}

printJob::~printJob()
{

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
