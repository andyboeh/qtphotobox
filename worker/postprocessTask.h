#ifndef _POSTPROCESSTASK_H
#define _POSTPROCESSTASK_H

#include <QPixmap>
#include <QList>

typedef enum {
    POSTPROCESS_TYPE_NONE = 0,
    POSTPROCESS_TYPE_SAVE_FULL_IMAGE,
    POSTPROCESS_TYPE_SAVE_ASSEMBLED_IMAGE,
} ePostprocessType;

class postprocessTask {
public:
    postprocessTask();
    postprocessTask(ePostprocessType type);
    void setTaskType(ePostprocessType type);
    ePostprocessType getTaskType();
    ~postprocessTask();
    void setPicture(QPixmap image);
    QPixmap getImage();
private:
    ePostprocessType mTaskType;
    QPixmap mImage;
    int mCopies;
};

#endif //_POSTPROCESSTASK_H
