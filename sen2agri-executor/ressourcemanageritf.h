#ifndef RESSOURCEMANAGERITF_H
#define RESSOURCEMANAGERITF_H

#include <QThread>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

#include <string>
using namespace std;

#include "iprocessorwrappermsgslistener.h"
#include "requestparamssubmitsteps.h"
#include "requestparamscanceltasks.h"
#include "requestparamsexecutioninfos.h"
#include "processorexecutioninfos.h"


/**
 * @brief The RessourceManagerItf class
 * \note
 * This class represents the interface with the Ressource Manager (SLURM).
 * It takes a wraper and executes the SLURM commands like srun, SSTAT, SACCT etc.)
 */
class RessourceManagerItf : public QThread, public IProcessorWrapperMsgsListener
{
    Q_OBJECT

public:
    ~RessourceManagerItf();

    static RessourceManagerItf *GetInstance();
    bool Start();
    void Stop();

    void StartProcessor(RequestParamsSubmitSteps *pReqParams);
    void CancelTasks(RequestParamsCancelTasks *pReqParams);

    virtual void OnProcessorNewMsg(RequestParamsBase *pReq);

private:
    RessourceManagerItf();
    void run();
    bool m_bStop;
    bool m_bStarted;

    QList<RequestParamsBase*> m_msgQueue;
    QMutex m_syncMutex;
    QWaitCondition m_condition;

    void AddRequestToQueue(RequestParamsBase *pReq);

    bool HandleStartProcessor(RequestParamsSubmitSteps *pReqParams);
    void HandleStopProcessor(RequestParamsCancelTasks *pReqParams);

    void HandleProcessorInfosMsg(RequestParamsExecutionInfos *pReqParams);
    // particular implementations for HandleProcessorInfosMsg
    void HandleProcessorExecutionStartedMsg(RequestParamsExecutionInfos *pReqParams);
    void HandleProcessorEndedMsg(RequestParamsExecutionInfos *pReqParams);
    void HandleProcessorInfosLogMsg(RequestParamsExecutionInfos *pReqParams);

    bool GetSacctResults(QString &sacctCmd, QList<ProcessorExecutionInfos> &procExecResults);
    QString BuildJobName(int nTaskId, QString &stepName, int idx);
    bool ParseJobName(const QString &jobName, int &nTaskId, QString &strStepName, int &nStepIdx);
    QString BuildParamsString(QStringList &listParams);

};

#endif // RESSOURCEMANAGERITF_H
