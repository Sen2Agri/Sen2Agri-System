#pragma once

#include "model.hpp"

#include "persistencemanager_interface.h"

class EventProcessingContext
{
    OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient;

public:
    EventProcessingContext(OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient);

    ConfigurationParameterValueList GetJobConfigurationParameters(int jobId, QString prefix);

    int SubmitTask(const NewTask &task);
    void SubmitSteps(int taskId, const NewStepList &steps);

    void MarkJobPaused(int jobId);
    void MarkJobResumed(int jobId);
    void MarkJobCancelled(int jobId);
    void MarkJobFinished(int jobId);
    void MarkJobFailed(int jobId);
    void MarkJobNeedsInput(int jobId);

    TaskIdList GetJobTasksByStatus(int jobId, const ExecutionStatusList &statusList);
    JobStepToRunList GetJobStepsForResume(int jobId);

    UnprocessedEventList GetNewEvents();
    void MarkEventProcessingStarted(int eventId);
    void MarkEventProcessingComplete(int eventId);
};
