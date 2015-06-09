#include "processorexecutioninfos.h"

/*static*/
QString ProcessorExecutionInfos::g_strRunning("running");

/*static*/
QString ProcessorExecutionInfos::g_strFinished("finished");

/*static*/
QString ProcessorExecutionInfos::g_strCanceled("cancelled");

ProcessorExecutionInfos::ProcessorExecutionInfos()
{

}

ProcessorExecutionInfos::~ProcessorExecutionInfos()
{

}

void ProcessorExecutionInfos::SetJobId(QString &strJobId)
{
    m_strJobId = strJobId;
}

void ProcessorExecutionInfos::SetJobName(QString &strJobName)
{
    m_strJobName = strJobName;
}

void ProcessorExecutionInfos::SetJobStatus(QString &strJobStatus)
{
    m_strJobStatus = strJobStatus;
}

void ProcessorExecutionInfos::SetStartTime(QString &strStartTime)
{
    m_strStartTime = strStartTime;
}

void ProcessorExecutionInfos::SetExecutionDuration(QString &strExecDuration)
{
    m_strExecutionDuration = strExecDuration;
}

void ProcessorExecutionInfos::SetCpuTime(QString &strCpuTime)
{
    m_strCpuTime = strCpuTime;
}

void ProcessorExecutionInfos::SetAveVmSize(QString &strAveVmSize)
{
    m_strAveVmSize = strAveVmSize;
}

void ProcessorExecutionInfos::SetMaxVmSize(QString &strMaxVmSize)
{
    m_strMaxVmSize = strMaxVmSize;
}

QString& ProcessorExecutionInfos::GetJobId()
{
    return m_strJobId;
}

QString& ProcessorExecutionInfos::GetJobName()
{
    return m_strJobName;
}

QString& ProcessorExecutionInfos::GetJobStatus()
{
    return m_strJobStatus;
}

QString& ProcessorExecutionInfos::GetStartTime()
{
    return m_strStartTime;
}

QString& ProcessorExecutionInfos::GetExecutionDuration()
{
    return m_strExecutionDuration;
}

QString& ProcessorExecutionInfos::GetCpuTime()
{
    return m_strCpuTime;
}

QString& ProcessorExecutionInfos::GetAveVmSize()
{
    return m_strAveVmSize;
}

QString& ProcessorExecutionInfos::GetMaxVmSize()
{
    return m_strMaxVmSize;
}
