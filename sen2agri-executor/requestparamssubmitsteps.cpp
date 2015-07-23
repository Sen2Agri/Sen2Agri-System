#include "requestparamssubmitsteps.h"

ExecutionStep::ExecutionStep()
{
}

ExecutionStep::ExecutionStep(int nTaskId, const QString &strName,
                             const QString &strProcPath)
{
    m_nTaskId = nTaskId;
    m_strStepName = strName;
    m_strProcessorPath = strProcPath;
}

ExecutionStep::~ExecutionStep()
{
}

void ExecutionStep::SetTaskId(int nTaskId)
{
    m_nTaskId = nTaskId;
}

void ExecutionStep::SetStepName(const QString &strName)
{
    m_strStepName = strName;
}

void ExecutionStep::SetProcessorPath(const QString &strProcPath)
{
    m_strProcessorPath = strProcPath;
}

void ExecutionStep::AddArgument(const QString &strArg)
{
    m_listArgs.push_back(strArg);
}

int ExecutionStep::GetTaskId()
{
    return m_nTaskId;
}

QString& ExecutionStep::GetStepName()
{
    return m_strStepName;
}

QString& ExecutionStep::GetProcessorPath()
{
    return m_strProcessorPath;
}

QStringList& ExecutionStep::GetArgumentsList()
{
    return m_listArgs;
}

ExecutionStep& ExecutionStep::operator=(const ExecutionStep &rhs)
{
    // Check for self-assignment!
    if (this == &rhs)      // Same object?
      return *this;

    m_nTaskId = rhs.m_nTaskId;
    m_strStepName = rhs.m_strStepName;
    m_strProcessorPath = rhs.m_strProcessorPath;
    m_listArgs = rhs.m_listArgs;
    return *this;
}

RequestParamsSubmitSteps::RequestParamsSubmitSteps()
    : RequestParamsBase(START_PROCESSOR_REQ)
{
}

ExecutionStep& RequestParamsSubmitSteps::AddExecutionStep(int nTaskId, const QString &strStepName,
        const QString &strProcPath)
{
    ExecutionStep execStep(nTaskId, strStepName, strProcPath);
    m_executionSteps.append(execStep);
    return m_executionSteps.last();
}

QList<ExecutionStep> &RequestParamsSubmitSteps::GetExecutionSteps()
{
    return m_executionSteps;
}
