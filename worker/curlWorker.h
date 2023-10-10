#ifndef _CURLWORKER_H
#define _CURLWORKER_H


#include "worker.h"

class curlWorker : public Worker
{
    Q_OBJECT
public:
    curlWorker();
    ~curlWorker();
public slots:
    void start();
    void stop();
    void sendAttachmentViaEmail(QString path);
};


#endif //_CURLWORKER_H
