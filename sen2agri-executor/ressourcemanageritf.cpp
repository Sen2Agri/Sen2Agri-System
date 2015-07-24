#include <stdio.h>
#include <QDateTime>
#include <QString>

#include <string>
using namespace std;

#include "ressourcemanageritf.h"
#include "commandinvoker.h"

#include "slurmsacctresultparser.h"
#include "processorwrapperfactory.h"
#include "logger.hpp"
#include "configurationmgr.h"
#include "persistenceitfmodule.h"
#include "orchestratorclient.h"

#include "requestparamssubmitsteps.h"
#include "requestparamscanceltasks.h"

//#define TEST
#ifdef TEST
QString SRUN_CMD("./SlurmSrunSimulator --job-name");
QString SACCT_CMD("./SlurmSrunSimulator "
                  "--format=JobID,JobName,NodeList,AveCPU,UserCPU,SystemCPU,ExitCode,AveVMSize,"
                  "MaxRSS,MaxVMSize,MaxDiskRead,MaxDiskWrite");
QString SCANCEL_CMD("scancel --name=");
#else

QString SRUN_FULL_CMD("srun --job-name");

QString SRUN_CMD("srun");
QString SRUN_PARAM("--job-name");

QString SACCT_CMD("sacct --parsable2 "
                  "--format=JobID,JobName,NodeList,AveCPU,UserCPU,SystemCPU,ExitCode,AveVMSize,"
                  "MaxRSS,MaxVMSize,MaxDiskRead,MaxDiskWrite");
QString SCANCEL_CMD("scancel --name=");
#endif

RessourceManagerItf::RessourceManagerItf()
{
    m_bStop = false;
    m_bStarted = false;
}

RessourceManagerItf::~RessourceManagerItf()
{
    Stop();
}

/*static*/
RessourceManagerItf *RessourceManagerItf::GetInstance()
{
    static RessourceManagerItf instance;
    return &instance;
}

bool RessourceManagerItf::Start()
{
    if (m_bStarted) {
        return true;
    }
    m_bStop = false;
    start();

    return true;
}

void RessourceManagerItf::Stop()
{
    m_bStop = true;
    m_condition.wakeAll();
    quit();
    wait();
}

void RessourceManagerItf::run()
{
    RequestParamsBase *pReqParams;
    bool bAvailable = false;

    while (!m_bStop) {
        pReqParams = 0;
        m_syncMutex.lock();
        m_condition.wait(&m_syncMutex);
        if (!m_msgQueue.isEmpty()) {
            pReqParams = m_msgQueue.takeFirst();
            bAvailable = true;
        } else {
            // we can have a stop command that only awakes the thread
            bAvailable = false;
        }
        m_syncMutex.unlock();

        while (bAvailable && pReqParams) {
            bAvailable = false;
            RequestType nMsgType = pReqParams->GetRequestType();
            switch (nMsgType) {
                case PROCESSOR_EXECUTION_INFO_MSG:
                    HandleProcessorInfosMsg((RequestParamsExecutionInfos *) pReqParams);
                    break;
                case START_PROCESSOR_REQ:
                    HandleStartProcessor((RequestParamsSubmitSteps *) pReqParams);
                    break;
                case STOP_PROCESSOR_REQ:
                    HandleStopProcessor((RequestParamsCancelTasks *) pReqParams);
                    break;
                default: // unknown msg type
                    break;
            }
            if (pReqParams)
                delete pReqParams;
            if (!m_msgQueue.isEmpty()) {
                pReqParams = m_msgQueue.takeFirst();
                bAvailable = true;
            }
        }
    }
}

/**
 * @brief RessourceManagerItf::StartProcessor
 */
void RessourceManagerItf::StartProcessor(RequestParamsSubmitSteps *pReqParams)
{
    AddRequestToQueue(pReqParams);
}

/**
 * @brief RessourceManagerItf::StopProcessor
 */
void RessourceManagerItf::CancelTasks(RequestParamsCancelTasks *pReqParams)
{
    AddRequestToQueue(pReqParams);
}

/**
 * @brief RessourceManagerItf::OnProcessorFinishedExecution
 * \note This function is called when a processor finished execution
 * It is called by the ExecutionInfosProtocolServer.
 */
void RessourceManagerItf::OnProcessorNewMsg(RequestParamsBase *pReq)
{
    AddRequestToQueue(pReq);
}

void RessourceManagerItf::AddRequestToQueue(RequestParamsBase *pReq)
{
    m_syncMutex.lock();
    m_msgQueue.push_back(pReq);
    m_syncMutex.unlock();
    m_condition.wakeAll();
}

/**
 * @brief RessourceManagerItf::StartProcessor
 */
bool RessourceManagerItf::HandleStartProcessor(RequestParamsSubmitSteps *pReqParams)
{
    // Get the path to the processor wrapper and pass to it the name of
    // the processor to be executed
    QString strProcWrpExecStr;
    QString str;
    QString strIpVal;
    QString strPortVal;
    QString strJobName;
    QList<ExecutionStep>::const_iterator stepIt;
    int i = 0;

    str = QString("PROCESSOR_WRAPPER_PATH");
    if (!ConfigurationMgr::GetInstance()->GetValue(str, strProcWrpExecStr)) {
        Logger::error(QString(
            "The path for the processor wrapper was not found. Please check configuration"));
        return false;
    }
    str = QString("SRV_IP_ADDR");
    ConfigurationMgr::GetInstance()->GetValue(str, strIpVal);
    str = QString("SRV_PORT_NO");
    ConfigurationMgr::GetInstance()->GetValue(str, strPortVal);

    QList<ExecutionStep> &execSteps = pReqParams->GetExecutionSteps();
    for (stepIt = execSteps.begin(); stepIt != execSteps.end(); stepIt++) {
        ExecutionStep executionStep = (*stepIt);
        strJobName = BuildJobName(executionStep.GetTaskId(), executionStep.GetStepName(), i);
        i++;
        // The following parameters are sent to the processor wrapper:
        //      PROC_PATH=<path to the processor>
        //      PROC_PARAMS=<parameters for the processor>
        //      JOB_NAME=<name of the slurm job> - this will be optional
        //      SRV_IP_ADDR=<IP address of this server> - optional
        //      SRV_PORT_NO=<port of this server> - optional

        QStringList listParams;
        QString strParam;
        listParams.push_back(SRUN_PARAM);
        listParams.push_back(strJobName);
        listParams.push_back(strProcWrpExecStr);

        strParam = QString("%1=%2").arg("SRV_IP_ADDR", strIpVal);
        listParams.append(strParam);
        strParam = QString("%1=%2").arg("SRV_PORT_NO", strPortVal);
        listParams.push_back(strParam);
        strParam = QString("%1=%2").arg("JOB_NAME", strJobName);
        listParams.push_back(strParam);

        // build the processor path to be execute along with its parameters
        // by adding also to the parameters list the processor name with its key (S2_PROC_NAME)
        strParam = QString("%1=%2").arg("PROC_PATH", executionStep.GetProcessorPath());
        listParams.push_back(strParam);

        if (!executionStep.GetArgumentsList().isEmpty()) {
            listParams.push_back("PROC_PARAMS");
            listParams.append(executionStep.GetArgumentsList());
        }

        QString paramsStr = BuildParamsString(listParams);
        // Build the srun command to be executed in SLURM - no need to wait
        //QString strSrunCmd = QString("%1 %2 %3").arg(SRUN_CMD, strJobName, paramsStr);

        Logger::debug(QString("HandleStartProcessor: Executing command %1 with params %2").arg(SRUN_CMD, paramsStr));

        CommandInvoker cmdInvoker;
        if (!cmdInvoker.InvokeCommand(SRUN_CMD, listParams, false)) {
            Logger::error(QString("Unable to execute SLURM srun command for the processor %1")
                              .arg(executionStep.GetProcessorPath()));
            return false;
        }

        // send the name of the job and the time to the persistence manager
        PersistenceItfModule::GetInstance()->MarkStepPendingStart(executionStep.GetTaskId(),
                                                                  executionStep.GetStepName());
    }

    return true;
}

/**
 * @brief RessourceManagerItf::StopProcessor
 */
void RessourceManagerItf::HandleStopProcessor(RequestParamsCancelTasks *pReqParams)
{
    QList<ProcessorExecutionInfos> procExecResults;
    if (GetSacctResults(SACCT_CMD, procExecResults)) {
        QList<int>::const_iterator idIt;
        QList<ProcessorExecutionInfos>::const_iterator procInfosIt;
        QString stepId;
        QList<int> listIds = pReqParams->GetTaskIdsToCancel();
        for (idIt = listIds.begin(); idIt != listIds.end(); idIt++) {
            stepId = QString("TSKID_%1_STEPNAME_").arg(*idIt);
            for (procInfosIt = procExecResults.begin(); procInfosIt != procExecResults.end();
                 procInfosIt++) {
                if ((*procInfosIt).strJobName.startsWith(stepId)) {
                    QString strCmd = QString("%1%2").arg(SCANCEL_CMD, (*procInfosIt).strJobName);
                    CommandInvoker cmdScancelInvoker;

                    Logger::debug(QString("HandleStopProcessor: Executing command %1").arg(strCmd));

                    // run scancel command and wait for it to return
                    if (!cmdScancelInvoker.InvokeCommand(strCmd, false)) {
                        // Log the execution trace here
                        Logger::error("Error executing SCANCEL command");
                    }

                    // TODO: See if this is really required or can be removed
                    //(*procInfosIt).strJobStatus = ProcessorExecutionInfos::g_strCanceled;
                    // Send the information about this job to the Persistence Manager
                    // PersistenceItfModule::GetInstance()->SendProcessorExecInfos((*procInfosIt));
                }
            }
        }
    }
}

void RessourceManagerItf::HandleProcessorInfosMsg(RequestParamsExecutionInfos *pReqParams)
{
    if (pReqParams->IsExecutionStarted()) {
        HandleProcessorExecutionStartedMsg(pReqParams);
    } else if (pReqParams->IsExecutionEnded()) {
        HandleProcessorEndedMsg(pReqParams);
    } else if (pReqParams->IsLogMsg()) {
        HandleProcessorInfosLogMsg(pReqParams);
    } else {
        Logger::info("Invalid message received from processor wrapper!");
    }
}

void RessourceManagerItf::HandleProcessorExecutionStartedMsg(
    RequestParamsExecutionInfos *pReqParams)
{
    // TODO: send the information to persistence manager
    // Send the statistic infos to the persistence interface module
    QString strJobName = pReqParams->GetJobName();
    int nTaskId = -1, nStepIdx = -1;
    QString strStepName;
    if (ParseJobName(strJobName, nTaskId, strStepName, nStepIdx)) {
        PersistenceItfModule::GetInstance()->MarkStepStarted(nTaskId, strStepName);
    } else {
        Logger::error(
            QString("Invalid job name for starting execution message %1").arg(strJobName));
    }
}

void RessourceManagerItf::HandleProcessorEndedMsg(RequestParamsExecutionInfos *pReqParams)
{
    // Get the job name and the execution time
    QString strJobName = pReqParams->GetJobName();
    QString executionDuration = pReqParams->GetExecutionTime();
    int nTaskId = -1, nStepIdx = -1;
    QString strStepName;

    // check if it a correct job name and extract the information from it
    if (ParseJobName(strJobName, nTaskId, strStepName, nStepIdx)) {
        Logger::debug(QString("HandleProcessorEndedMsg: Executing command %1").arg(SACCT_CMD));

        CommandInvoker cmdInvoker;
        if (cmdInvoker.InvokeCommand(SACCT_CMD, false)) {
            QString strLog = cmdInvoker.GetExecutionLog();
            SlurmSacctResultParser slurmSacctParser;
            QList<ProcessorExecutionInfos> procExecResults;
            if (slurmSacctParser.ParseResults(strLog, procExecResults, &strJobName) > 0) {
                ProcessorExecutionInfos jobExecInfos = procExecResults.at(0);
                jobExecInfos.strExecutionDuration = executionDuration;
                jobExecInfos.strJobStatus = ProcessorExecutionInfos::g_strFinished;
                // Send the statistic infos to the persistence interface module
                if (PersistenceItfModule::GetInstance()->MarkStepFinished(nTaskId, strStepName,
                                                                          jobExecInfos)) {
                    OrchestratorClient().NotifyEventsAvailable();
                }

            } else {
                Logger::error(
                    QString("Unable to parse SACCT results for job name %1").arg(strJobName));
            }
        }
    } else {
        Logger::error(
            QString("Invalid job name for starting execution message %1").arg(strJobName));
    }
}

void RessourceManagerItf::HandleProcessorInfosLogMsg(RequestParamsExecutionInfos *pReqParams)
{
    // just send the information to the Logger
    Logger::info(
        QString("JOB_NAME: %1, MSG: %2").arg(pReqParams->GetJobName(), pReqParams->GetLogMsg()));
}

bool RessourceManagerItf::GetSacctResults(QString &sacctCmd,
                                          QList<ProcessorExecutionInfos> &procExecResults)
{
    CommandInvoker cmdInvoker;
    if (cmdInvoker.InvokeCommand(sacctCmd, false)) {
        QString strLog = cmdInvoker.GetExecutionLog();
        SlurmSacctResultParser slurmSacctParser;
        if (slurmSacctParser.ParseResults(strLog, procExecResults, NULL) > 0) {
            return true;
        } else {
            Logger::error(QString("Unable to parse SACCT results"));
        }
    }
    return false;
}

QString RessourceManagerItf::BuildJobName(int nTaskId, QString &stepName, int idx)
{
    return QString("TSKID_%1_STEPNAME_%2_%3")
        .arg(QString::number(nTaskId), stepName, QString::number(idx));
}

bool RessourceManagerItf::ParseJobName(const QString &jobName,
                                       int &nTaskId,
                                       QString &strStepName,
                                       int &nStepIdx)
{
    // The expected format is "TSKID_%1_STEPNAME_%2_%3"
    QStringList list = jobName.split('_');
    int listSize = list.size();
    if (listSize >= 5) {
        if (list.at(0) == "TSKID" && list.at(2) == "STEPNAME") {
            bool bTaskOk = false;
            bool bStepOk = false;
            nTaskId = list.at(1).toInt(&bTaskOk);
            nStepIdx = list.at(list.size() - 1).toInt(&bStepOk);
            if (bTaskOk && bStepOk) {
                strStepName = list.at(3);
                if (listSize > 5) {
                    // we might have the step name built from several items separated by _
                    // compute the number of additional components
                    int nStepNameComponentsCnt = list.size() - 5;

                    for (int i = 0; i < nStepNameComponentsCnt; i++) {
                        strStepName.append('_');
                        strStepName.append(list.at(4 + i));
                    }
                }
                return true;
            }
        }
    }
    return false;
}

QString RessourceManagerItf::BuildParamsString(QStringList &listParams)
{
    QString retStr;
    for (QStringList::iterator it = listParams.begin(); it != listParams.end(); ++it) {
        retStr.append(*it);
        retStr.append(" ");
    }
    return retStr;
}
