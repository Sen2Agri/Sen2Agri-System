#include "ressourcemanageritf.h"
#include "commandinvoker.h"
#include <string>
using namespace std;

#include <stdio.h>

#include <QDateTime>
#include <QString>

#include "slurmsacctresultparser.h"
#include "processorexecutioninfos.h"
#include "processorwrapperfactory.h"
#include "logger.h"
#include "configurationmgr.h"
#include "persistenceitfmodule.h"

#define TEST
#ifdef TEST
QString SRUN_CMD("../../dist/SlurmSrunSimulator --job-name");
QString SACCT_CMD("../../dist/SlurmSrunSimulator --format=JobID,Jobname,CPUTimeRAW,TotalCPU,MaxVMSize");
QString SSTAT_CMD("../../dist/SlurmSrunSimulator --format=JobID,Jobname,CPUTimeRAW,TotalCPU,MaxVMSize");
QString SCANCEL_CMD("scancel");
#else
QString SRUN_CMD("srun --job-name");
QString SACCT_CMD("sacct --format=JobID,Jobname,AveCPU,AveVMSize,MaxVMSize");
QString SSTAT_CMD("sstat --format=JobID,Jobname,AveCPU,AveVMSize,MaxVMSize");
QString SCANCEL_CMD("scancel");
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
    if(m_bStarted)
    {
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
    QVariantMap reqParams;
    bool bAvailable = false;

    while(!m_bStop)
    {
        m_syncMutex.lock();
        m_condition.wait(&m_syncMutex);
        if(!m_msgQueue.isEmpty()) {
            reqParams = m_msgQueue.takeFirst();
            bAvailable = true;
        } else {
            // we can have a stop command that only awakes the thread
            bAvailable = false;
        }
        m_syncMutex.unlock();

        while(bAvailable) {
            bAvailable = false;
            int nMsgType = reqParams["MSG_TYPE"].toInt();
            switch (nMsgType) {
                case PROCESSOR_ENDED:
                    HandleProcessorEndedMsg(reqParams);
                    break;
                case PROCESSOR_INFO_MSG:
                    HandleProcessorInfosMsg(reqParams);
                    break;
                case START_PROCESSOR_REQ:
                    HandleStartProcessor(reqParams);
                    break;
                case STOP_PROCESSOR_REQ:
                    HandleStopProcessor(reqParams);
                    break;
                default:    // unknown msg type
                    break;
            }
            if(!m_msgQueue.isEmpty()) {
                reqParams = m_msgQueue.takeFirst();
                bAvailable = true;
            }
        }
    }
}

/**
 * @brief RessourceManagerItf::StartProcessor
 */
bool RessourceManagerItf::StartProcessor(QVariantMap &reqParams)
{
    int nMsgType = reqParams["MSG_TYPE"].toInt();
    if(nMsgType == START_PROCESSOR_REQ) {
        AddRequestToQueue(reqParams);
        return true;
    }
    return false;
}

/**
 * @brief RessourceManagerItf::StopProcessor
 */
bool RessourceManagerItf::StopProcessor(QVariantMap &reqParams)
{
    int nMsgType = reqParams["MSG_TYPE"].toInt();
    if(nMsgType == STOP_PROCESSOR_REQ) {
        AddRequestToQueue(reqParams);
        return true;
    }
    return false;
}

/**
 * @brief RessourceManagerItf::OnProcessorFinishedExecution
 * \note This function is called when a processor finished execution
 * It is called by the ExecutionInfosProtocolServer.
 */
void RessourceManagerItf::OnProcessorNewMsg(QVariantMap &msgVals)
{
    AddRequestToQueue(msgVals);
}

void RessourceManagerItf::AddRequestToQueue(QVariantMap &req)
{
    m_syncMutex.lock();
    m_msgQueue.append(req);
    m_syncMutex.unlock();
    m_condition.wakeAll();
}

/**
 * @brief RessourceManagerItf::StartProcessor
 */
bool RessourceManagerItf::HandleStartProcessor(QVariantMap &reqParams)
{
    // Get the path to the processor wrapper and pass to it the name of
    // the processor to be executed
    QString strProcWrpExecStr;
    QString str;

    str = QString("PROCESSOR_WRAPPER_PATH");
    if(!ConfigurationMgr::GetInstance()->GetValue(str, strProcWrpExecStr))
    {
        Logger::GetInstance()->error("The path for the processor wrapper was not found. Please check configuration");
        return false;
    }
    // The following parameters are sent to the processor wrapper:
    //      PROC_PATH=<path to the processor>
    //      PROC_PARAMS=<parameters for the processor>
    //      JOB_NAME=<name of the slurm job> - this will be optional
    //      SRV_IP_ADDR=<IP address of this server> - optional
    //      SRV_PORT_NO=<port of this server> - optional

    QString strProcName = reqParams["PROC_NAME"].toString();
    QString strParams = reqParams["PROC_ARGS"].toString();
    QString strProcPath;
     if(!ProcessorWrapperFactory::GetInstance()->GetProcessorPath(strProcName, strProcPath))
     {
         Logger::GetInstance()->error("Cannot find the path for the processor %s",
                                      strProcName.toStdString().c_str());
         return false;
     }

    // build the processor path to be execute along with its parameters
    // by adding also to the parameters list the processor name with its key (S2_PROC_NAME)
    strProcWrpExecStr.append(" PROC_PATH=\"");
    strProcWrpExecStr.append(strProcPath);
    strProcWrpExecStr.append("\"");
    if(!strParams.isEmpty()) {
        strProcWrpExecStr.append(" PROC_PARAMS=\"").append(strParams).append("\"");
    }

    // build the job name as yyyyMMddhhmmsszzz_PROCESSOR_NAME
    QString formattedTime = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz_");
    QString strJobName = formattedTime.append(strProcName);

    strProcWrpExecStr.append(" JOB_NAME=\"").append(strJobName).append("\"");

    QString strIpVal;
    QString strPortVal;
    str = QString("PROT_SRV_IP_ADDR");
    ConfigurationMgr::GetInstance()->GetValue(str, strIpVal);
    str = QString("PROT_SRV_PORT");
    ConfigurationMgr::GetInstance()->GetValue(str, strPortVal);

    strProcWrpExecStr.append(" SRV_IP_ADDR=\"").append(strIpVal).append("\"");
    strProcWrpExecStr.append(" SRV_PORT_NO=\"").append(strPortVal).append("\"");

    // Build the srun command to be executed in SLURM - no need to wait
    QString strSrunCmd = QString("%1 %2 %3 &").arg(SRUN_CMD, strJobName, strProcWrpExecStr);;

    CommandInvoker cmdInvoker;
    if(!cmdInvoker.InvokeCommand(strSrunCmd, false))
    {
        Logger::GetInstance()->error("Unable to execute SLURM srun command for the processor %s and parameters %s",
                                    strProcName.toStdString().c_str(), strParams.toStdString().c_str());
        return false;
    }

    // send the name of the job and the time to the persistence manager
    ProcessorExecutionInfos infos;
    infos.SetJobName(strJobName);
    QDateTime curDateTime = QDateTime::currentDateTime();
    QString strCurDate = curDateTime.toString("dd/MM/yyyy hh:mm:ss");
    infos.SetStartTime(strCurDate);
    PersistenceItfModule::GetInstance()->SendProcessorStart(infos);

    return true;
}

/**
 * @brief RessourceManagerItf::StopProcessor
 */
void RessourceManagerItf::HandleStopProcessor(QVariantMap &reqParams)
{
    // Get the job id based on the given job name
    // The command sstat is used to extract the  jobid
    CommandInvoker cmdInvoker;
    if(cmdInvoker.InvokeCommand(SSTAT_CMD, false))
    {
        QString strLog = cmdInvoker.GetExecutionLog();
        SlurmSacctResultParser slurmSacctParser;
        QList<ProcessorExecutionInfos> procExecResults;
        QString strJobName = reqParams["PROC_JOB_NAME"].toString();
        if(slurmSacctParser.ParseResults(strLog, procExecResults, &strJobName) > 0)
        {
            // Build the scancel command.
            // The jobId is then used for scancel
            ProcessorExecutionInfos infos = procExecResults.at(0);
            QString strCmd = QString("%1 %2").arg(SCANCEL_CMD, infos.GetJobId());

            CommandInvoker cmdScancelInvoker;
            // run scancel command but without waiting
            if(!cmdScancelInvoker.InvokeCommand(strCmd, true)) {
                // Log the execution trace here
                Logger::GetInstance()->error("Error executing SCANCEL command");
            }
            // Send the information about this job to the Persistence Manager
            PersistenceItfModule::GetInstance()->SendProcessorCancel(infos);
        }
    } else {
        Logger::GetInstance()->error("Error executing SSTAT command");
    }
}

bool RessourceManagerItf::HandleProcessorEndedMsg(QVariantMap &msgVals)
{
    // A json message will be expected with the following format:
    // {
    //      JOB_NAME : <name of the slurm job as originally created>
    //      EXEC_TIME : <duration of the execution of the processor>
    // }
    // Get the job identifier
    QString jobName = msgVals["JOB_NAME"].toString();
    QString executionTime = msgVals["EXEC_TIME"].toString();

    CommandInvoker cmdInvoker;
    if(cmdInvoker.InvokeCommand(SACCT_CMD, false))
    {
        QString strLog = cmdInvoker.GetExecutionLog();
        SlurmSacctResultParser slurmSacctParser;
        QList<ProcessorExecutionInfos> procExecResults;
        if(slurmSacctParser.ParseResults(strLog, procExecResults, &jobName) > 0)
        {
            ProcessorExecutionInfos jobExecInfos = procExecResults.at(0);
            jobExecInfos.SetExecutionTime(executionTime);
            // Send the statistic infos to the persistence interface module
            PersistenceItfModule::GetInstance()->SendProcessorEnd(jobExecInfos);
            return true;
        } else {
            Logger::GetInstance()->error("Unable to parse SACCT results for job name %s", jobName.toStdString().c_str());
        }
    }
    return false;
}

bool RessourceManagerItf::HandleProcessorInfosMsg(QVariantMap &msgVals)
{
    // A json message will be expected with the following format:
    // {
    //      JOB_NAME : <name of the slurm job as originally created>
    //      LOG : <the log message>
    // }

    // just send the information to the Logger
    Logger::GetInstance()->info("JOB_NAME: %s, MSG: %s",
                                msgVals["JOB_NAME"].toString().toStdString().c_str(),
                                msgVals["LOG"].toString().toStdString().c_str());
    return true;
}

