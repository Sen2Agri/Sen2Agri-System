#ifndef PROCESSOREXECUTIONINFOS_H
#define PROCESSOREXECUTIONINFOS_H

#include <QString>

class ProcessorExecutionInfos
{
public:
    ProcessorExecutionInfos() {
        strJobId = "N/A";
        strJobName = "N/A";
        strJobNode = "N/A";
        strExitCode = "N/A";
        strJobStatus = "N/A";
        strStartTime = "N/A";
        strExecutionDuration = "N/A";
        strCpuTime = "N/A";
        strUserTime = "N/A";
        strSystemTime = "N/A";
        strAveVmSize = "N/A";
        strMaxRss = "N/A";
        strMaxVmSize = "N/A";
        strDiskRead = "N/A";
        strDiskWrite = "N/A";
    }

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

    QString strStdOutText;
    QString strStdErrText;
};

#endif // PROCESSOREXECUTIONINFOS_H
