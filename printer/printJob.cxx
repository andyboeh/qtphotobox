#include "printJob.h"

printJob::printJob()
{
    mCopies = 0;
}

printJob::printJob(QPixmap image)
{
    mCopies = 1;
    mImage = image;
}

printJob::printJob(QPixmap image, int copies)
{
    mImage = image;
    mCopies = copies;
}

printJob::printJob(QString filename, int copies)
{
    mFilename = filename;
    mCopies = copies;
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

QPixmap printJob::getImage() {
    return mImage;
}

QString printJob::getFile()
{
    return mFilename;
}
