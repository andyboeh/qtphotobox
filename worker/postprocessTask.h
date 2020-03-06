#ifndef _POSTPROCESSTASK_H
#define _POSTPROCESSTASK_H

#include <QPixmap>
#include <QList>

class postprocessTask {
public:
    typedef enum {
        POSTPROCESS_TYPE_NONE = 0,
        POSTPROCESS_TYPE_SAVE_FULL_IMAGE,
        POSTPROCESS_TYPE_SAVE_ASSEMBLED_IMAGE,
        POSTPROCESS_TYPE_CREATE_THUMBNAIL
    } ePostprocessType;

    postprocessTask();
    postprocessTask(ePostprocessType type);
    void setTaskType(ePostprocessType type);
    ePostprocessType getTaskType();
    ~postprocessTask();
    void setPicture(QPixmap image);
    void setFile(QString filename);
    QPixmap getImage();
    QString getFile();
private:
    QString mFilename;
    ePostprocessType mTaskType;
    QPixmap mImage;
    int mCopies;
};

#endif //_POSTPROCESSTASK_H
