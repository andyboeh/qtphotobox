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

printJob::~printJob()
{

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
