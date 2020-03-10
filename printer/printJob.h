#ifndef _PRINTJOB_H
#define _PRINTJOB_H

#include <QPixmap>
#include <QString>

class printJob {
public:
    printJob();
    printJob(QPixmap image);
    printJob(QPixmap image, int copies);
    printJob(QString filename, int copies);
    ~printJob();
    bool isFileJob();
    void setCopies(int copies);
    int getCopies();
    int getCopiesPrinted();
    void setCopiesPrinted(int copies);
    QPixmap getImage();
    QString getFile();
    int getWidth();
    int getHeight();
private:
    int mCopiesPrinted;
    QString mFilename;
    QPixmap mImage;
    int mHeight;
    int mWidth;
    int mCopies;
};

#endif //_PRINTJOB_H
