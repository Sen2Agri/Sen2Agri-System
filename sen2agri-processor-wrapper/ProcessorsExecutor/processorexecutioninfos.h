#ifndef PROCESSOREXECUTIONINFOS_H
#define PROCESSOREXECUTIONINFOS_H

#include <QString>

class ProcessorExecutionInfos
{
public:
    ProcessorExecutionInfos();
    ~ProcessorExecutionInfos();

    void SetJobId(QString &strJobId);
    void SetJobName(QString &strJobName);
    void SetJobStatus(QString &strJobStatus);
    void SetStartTime(QString &strStartTime);
    void SetExecutionDuration(QString &strExecDuration);
    void SetCpuTime(QString& strCpuTime);
    void SetAveVmSize(QString& strAveVmSize);
    void SetMaxVmSize(QString& strMaxVmSize);

    QString& GetJobName();
    QString& GetJobId();
    QString& GetJobStatus();
    QString& GetStartTime();
    QString& GetExecutionDuration();
    QString& GetCpuTime();
    QString& GetAveVmSize();
    QString& GetMaxVmSize();

    static QString g_strRunning;
    static QString g_strFinished;
    static QString g_strCanceled;

private:
    QString m_strJobId;
    QString m_strJobName;
    QString m_strJobStatus;
    QString m_strStartTime;
    QString m_strExecutionDuration;
    QString m_strCpuTime;
    QString m_strAveVmSize;
    QString m_strMaxVmSize;
};

#endif // PROCESSOREXECUTIONINFOS_H
