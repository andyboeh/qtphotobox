#ifndef _PRINTJOB_H
#define _PRINTJOB_H

#include <QPixmap>

class printJob {
public:
    printJob();
    printJob(QPixmap image);
    printJob(QPixmap image, int copies);
    ~printJob();
    void setCopies(int copies);
    int getCopies();
    QPixmap getImage();
private:
    QPixmap mImage;
    int mCopies;
};

#endif //_PRINTJOB_H
