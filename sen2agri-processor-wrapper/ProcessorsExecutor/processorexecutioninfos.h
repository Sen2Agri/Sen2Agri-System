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
    void SetStartTime(QString &strStartTime);
    void SetExecutionTime(QString &strExecTime);
    void SetCpuTime(QString& strCpuTime);
    void SetAveVmSize(QString& strAveVmSize);
    void SetMaxVmSize(QString& strMaxVmSize);

    QString& GetJobName();
    QString& GetJobId();
    QString& GetStartTime();
    QString& GetExecutionTime();
    QString& GetCpuTime();
    QString& GetAveVmSize();
    QString& GetMaxVmSize();


private:
    QString m_strJobId;
    QString m_strJobName;
    QString m_strStartTime;
    QString m_strExecutionTime;
    QString m_strCpuTime;
    QString m_strAveVmSize;
    QString m_strMaxVmSize;
};

#endif // PROCESSOREXECUTIONINFOS_H
