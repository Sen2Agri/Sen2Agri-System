#include "processorexecutioninfos.h"

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

void ProcessorExecutionInfos::SetStartTime(QString &strStartTime)
{
    m_strStartTime = strStartTime;
}

void ProcessorExecutionInfos::SetExecutionTime(QString &strExecTime)
{
    m_strExecutionTime = strExecTime;
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

QString& ProcessorExecutionInfos::GetStartTime()
{
    return m_strStartTime;
}

QString& ProcessorExecutionInfos::GetExecutionTime()
{
    return m_strExecutionTime;
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
