#ifndef PROCESSOREXECUTIONINFOS_H
#define PROCESSOREXECUTIONINFOS_H

#include <QString>

class ProcessorExecutionInfos
{
public:
    static QString g_strRunning;
    static QString g_strFinished;
    static QString g_strCanceled;

    QString strJobId;
    QString strJobName;
    QString strJobNode;
    QString strExitCode;
    QString strJobStatus;
    QString strStartTime;
    QString strExecutionDuration;
    QString strCpuTime;
    QString strUserTime;
    QString strSystemTime;
    QString strAveVmSize;
    QString strMaxRss;
    QString strMaxVmSize;
    QString strDiskRead;
    QString strDiskWrite;
};

#endif // PROCESSOREXECUTIONINFOS_H
