#ifndef REQUESTPARAMSCANCELTASKS_H
#define REQUESTPARAMSCANCELTASKS_H


#include "requestparamsbase.h"
#include <QList>

class RequestParamsCancelTasks : public RequestParamsBase
{
public:
    RequestParamsCancelTasks();

    void SetTaskIdsToCancel(QList<int> &listIds);
    const QList<int>& GetTaskIdsToCancel();

private:
    QList<int> m_taskIdsToCancel;
};

#endif // REQUESTPARAMSCANCELTASKS_H
