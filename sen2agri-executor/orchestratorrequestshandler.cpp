#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "orchestratorrequestshandler.h"
#include "ressourcemanageritf.h"
#include "requestparamssubmitsteps.h"
#include "requestparamscanceltasks.h"

void OrchestratorRequestsHandler::SubmitSteps(const NewExecutorStepList &steps)
{
    QList<NewExecutorStep>::const_iterator stepIt;
    QList<StepArgument>::const_iterator stepArgIt;

    RequestParamsSubmitSteps *pReqParams = new RequestParamsSubmitSteps();

    for (stepIt = steps.begin(); stepIt != steps.end(); stepIt++) {
        ExecutionStep& execStep = pReqParams->AddExecutionStep((*stepIt).taskId, (*stepIt).stepName, (*stepIt).processorPath);
        for (stepArgIt = (*stepIt).arguments.begin(); stepArgIt != (*stepIt).arguments.end(); stepArgIt++) {
            execStep.AddArgument((*stepArgIt).value);
        }
    }
    RessourceManagerItf::GetInstance()->StartProcessor(pReqParams);
}

void OrchestratorRequestsHandler::CancelTasks(const TaskIdList &tasks)
{
    RequestParamsCancelTasks *pReq = new RequestParamsCancelTasks();
    QList<int> taskIds;
    QList<int>::const_iterator idIt;
    for (idIt = tasks.begin(); idIt != tasks.end(); idIt++) {
        taskIds.append(*idIt);
    }
    pReq->SetTaskIdsToCancel(taskIds);
    RessourceManagerItf::GetInstance()->CancelTasks(pReq);
}

