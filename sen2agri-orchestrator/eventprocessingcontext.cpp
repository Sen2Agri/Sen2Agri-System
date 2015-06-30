#include "eventprocessingcontext.hpp"
#include "dbus_future_utils.hpp"

EventProcessingContext::EventProcessingContext(
    OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient)
    : persistenceManagerClient(persistenceManagerClient), rescanRequested()
{
}

void EventProcessingContext::ScheduleRescan()
{
    rescanRequested = true;
}

void EventProcessingContext::CancelRescan()
{
    rescanRequested = false;
}

bool EventProcessingContext::IsRescanRequested() const
{
    return rescanRequested;
}

ConfigurationParameterValueList
EventProcessingContext::GetJobConfigurationParameters(int jobId, QString prefix)
{
    return WaitForResponseAndThrow(
        persistenceManagerClient.GetJobConfigurationParameters(jobId, prefix));
}

int EventProcessingContext::SubmitTask(const NewTask &task)
{
    return WaitForResponseAndThrow(persistenceManagerClient.SubmitTask(task));
}

void EventProcessingContext::SubmitSteps(int taskId, const NewStepList &steps)
{
    WaitForResponseAndThrow(persistenceManagerClient.SubmitSteps(taskId, steps));
}

void EventProcessingContext::MarkJobPaused(int jobId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkJobPaused(jobId));
}

void EventProcessingContext::MarkJobResumed(int jobId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkJobResumed(jobId));
}

void EventProcessingContext::MarkJobCancelled(int jobId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkJobCancelled(jobId));
}

void EventProcessingContext::MarkJobFinished(int jobId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkJobFinished(jobId));
}

void EventProcessingContext::MarkJobFailed(int jobId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkJobFailed(jobId));
}

void EventProcessingContext::MarkJobNeedsInput(int jobId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkJobNeedsInput(jobId));
}

TaskIdList EventProcessingContext::GetJobTasksByStatus(int jobId,
                                                       const ExecutionStatusList &statusList)
{
    return WaitForResponseAndThrow(persistenceManagerClient.GetJobTasksByStatus(jobId, statusList));
}

UnprocessedEventList EventProcessingContext::GetNewEvents()
{
    return WaitForResponseAndThrow(persistenceManagerClient.GetNewEvents());
}

void EventProcessingContext::MarkEventProcessingStarted(int eventId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkEventProcessingStarted(eventId));
}

void EventProcessingContext::MarkEventProcessingComplete(int eventId)
{
    WaitForResponseAndThrow(persistenceManagerClient.MarkEventProcessingComplete(eventId));
}
