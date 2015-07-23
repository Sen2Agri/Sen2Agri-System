#include "requestparamscanceltasks.h"

RequestParamsCancelTasks::RequestParamsCancelTasks()
    : RequestParamsBase(STOP_PROCESSOR_REQ)
{

}

void RequestParamsCancelTasks::SetTaskIdsToCancel(QList<int> &listIds)
{
    m_taskIdsToCancel.clear();
    m_taskIdsToCancel = listIds;
}

const QList<int>& RequestParamsCancelTasks::GetTaskIdsToCancel()
{
    return m_taskIdsToCancel;
}
