#ifndef SUBMITSTEPSREQUESTPARAMS_H
#define SUBMITSTEPSREQUESTPARAMS_H


#include "requestparamsbase.h"

class ExecutionStep
{
public:
    ExecutionStep();
    ExecutionStep(int nTaskId, const QString &strName, const QString &strQos, const QString &strPartition, const QString &strProcPath);
    ~ExecutionStep();

public:
    void SetTaskId(int nTaskId);
    void SetStepName(const QString &strName);
    void SetQos(const QString &strQos);
    void SetPartition(const QString &strPartition);
    void SetProcessorPath(const QString &strProcPath);
    void AddArgument(const QString &strArg);

    int GetTaskId();
    QString &GetStepName();
    QString &GetQos();
    QString &GetPartition();
    QString &GetProcessorPath();
    QStringList &GetArgumentsList();

    ExecutionStep& operator=(const ExecutionStep &rhs);

private:
    int m_nTaskId;
    QString m_strStepName;
    QString m_strQos;
    QString m_strPartition;
    QString m_strProcessorPath;
    QStringList m_listArgs;
};

class RequestParamsSubmitSteps : public RequestParamsBase
{
public:
    RequestParamsSubmitSteps();

    ExecutionStep &AddExecutionStep(int nTaskId, const QString &strStepName,
                                    const QString &strQos, const QString &strPartition,
                                    const QString &strProcPath);
    QList<ExecutionStep> & GetExecutionSteps();

private:
    QList<ExecutionStep> m_executionSteps;
};

#endif // SUBMITSTEPSREQUESTPARAMS_H
