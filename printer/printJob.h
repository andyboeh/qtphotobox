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
    QPixmap getImage();
    QString getFile();
private:
    QString mFilename;
    QPixmap mImage;
    int mCopies;
};

#endif //_PRINTJOB_H
