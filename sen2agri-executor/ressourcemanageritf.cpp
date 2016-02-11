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

QString SRUN_CMD("srun");
QString SRUN_JOB_NAME_PARAM("--job-name");
QString SRUN_QOS_PARAM("--qos");
QString SRUN_PARTITION_PARAM("--partition");

QString SACCT_CMD("sacct");

QString SACCT_CMD_ARG1("--parsable2");
QString
    SACCT_CMD_ARG2("--format=JobID,JobName,NodeList,AveCPU,UserCPU,SystemCPU,ExitCode,AveVMSize,"
                   "MaxRSS,MaxVMSize,MaxDiskRead,MaxDiskWrite");

QString SCANCEL_CMD("scancel");
QString SCANCEL_CMD_ARGS("--name");

QString SBATCH_CMD("sbatch");
#endif

#define SACCT_RETRY_CNT                 10
#define SACCT_RETRY_TIMEOUT_IN_MSEC     1000

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
        QString strQos = PersistenceItfModule::GetInstance()->GetExecutorQos(executionStep.GetProcessorId());
        QString strPartition = PersistenceItfModule::GetInstance()->GetExecutorPartition(executionStep.GetProcessorId());
        if(!strQos.isEmpty()) {
            listParams.push_back(SRUN_QOS_PARAM);
            listParams.push_back(strQos);

        }
        if(!strPartition.isEmpty()) {
            listParams.push_back(SRUN_PARTITION_PARAM);
            listParams.push_back(strPartition);
        }
        listParams.push_back(SRUN_JOB_NAME_PARAM);
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
        // QString strSrunCmd = QString("%1 %2 %3").arg(SRUN_CMD, strJobName, paramsStr);

        Logger::debug(QString("HandleStartProcessor: Executing command %1 with params %2")
                          .arg(SRUN_CMD, paramsStr));

        QTemporaryFile tempFile;
        if (tempFile.open()) {
            // TODO: The simplest way would be to send an inline shell script but I don't know why it doesn't work
            //QString cmd = QString(" <<EOF\n#!/bin/sh\nsrun %1\nEOF").arg(paramsStr);
            QString cmd = QString("#!/bin/sh\nsrun %1").arg(paramsStr);
           tempFile.write(cmd.toStdString().c_str(), cmd.length());
           tempFile.flush();
           if(!tempFile.setPermissions(QFile::ReadOwner|
                                       QFile::WriteOwner|
                                       QFile::ExeOwner|
                                       QFile::ReadGroup|
                                       QFile::ExeGroup|
                                       QFile::ReadOther|QFile::ExeOther)) {
               Logger::error(QString("Unable to execute SLURM sbatch command for the processor %1. "
                                     "The execution permissions cannot be set for the script!")
                                 .arg(executionStep.GetProcessorPath()));
               return false;
           }
        } else {
            Logger::error(QString("Unable to execute SLURM sbatch command for the processor %1. "
                                  "The script cannot be created!")
                              .arg(executionStep.GetProcessorPath()));
            return false;
        }
        QStringList sbatchParams;
        //sbatchParams.push_back(QString(" <<EOF\n#!/bin/sh\nsrun %1\nEOF").arg(paramsStr));
        sbatchParams.push_back(tempFile.fileName());

        Logger::debug(QString("HandleStartProcessor: Executing command %1 with params %2")
                          .arg(SBATCH_CMD, sbatchParams.at(0)));

        CommandInvoker cmdInvoker;
        if (!cmdInvoker.InvokeCommand(SBATCH_CMD, sbatchParams, false)) {
            Logger::error(QString("Unable to execute SLURM sbatch command for the processor %1. The error was: \"%s\"")
                              .arg(executionStep.GetProcessorPath(), cmdInvoker.GetExecutionLog()));
            return false;
        }
        Logger::debug(QString("HandleStartProcessor: Sbatch command returned: \"%1\"")
                          .arg(cmdInvoker.GetExecutionLog()));

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
                    CommandInvoker cmdScancelInvoker;

                    // run scancel command and wait for it to return
                    QStringList args;
                    args << SCANCEL_CMD_ARGS << (*procInfosIt).strJobName;

                    Logger::debug(QString("HandleStopProcessor: Executing command %1 %2")
                                      .arg(SCANCEL_CMD)
                                      .arg(args.join(' ')));

                    if (!cmdScancelInvoker.InvokeCommand(SCANCEL_CMD, args, false)) {
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
        CommandInvoker cmdInvoker;
        QStringList args;
        args << SACCT_CMD_ARG1 << SACCT_CMD_ARG2;

        Logger::debug(QString("HandleProcessorEndedMsg: Executing command %1 %2")
                          .arg(SACCT_CMD)
                          .arg(args.join(' ')));

        int retryCnt = 0;
        ProcessorExecutionInfos jobExecInfos;
        jobExecInfos.strJobName = strJobName;
        while(retryCnt < SACCT_RETRY_CNT) {
            if (cmdInvoker.InvokeCommand(SACCT_CMD, args, false)) {
                QString strLog = cmdInvoker.GetExecutionLog();
                SlurmSacctResultParser slurmSacctParser;
                QList<ProcessorExecutionInfos> procExecResults;
                if (slurmSacctParser.ParseResults(strLog, procExecResults, &strJobName) > 0) {
                    jobExecInfos = procExecResults.at(0);
                    break;
                } else {
                    Logger::error(
                        QString("Unable to parse SACCT results for job name %1. Retrying ...").arg(strJobName));
                    msleep(SACCT_RETRY_TIMEOUT_IN_MSEC);
                }
            } else {
                Logger::error(
                    QString("Error executing SACCT for job name %1").arg(strJobName));
                break;
            }
            retryCnt++;
        }
        jobExecInfos.strExecutionDuration = executionDuration;
        jobExecInfos.strJobStatus = ProcessorExecutionInfos::g_strFinished;
        jobExecInfos.strStdOutText = pReqParams->GetStdOutText();
        jobExecInfos.strStdErrText = pReqParams->GetStdErrText();
        // Send the statistic infos to the persistence interface module
        if (PersistenceItfModule::GetInstance()->MarkStepFinished(nTaskId, strStepName,
                                                                  jobExecInfos)) {
            OrchestratorClient().NotifyEventsAvailable();
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

    QStringList args;
    args << SACCT_CMD_ARG1 << SACCT_CMD_ARG2;

    if (cmdInvoker.InvokeCommand(sacctCmd, args, false)) {
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
