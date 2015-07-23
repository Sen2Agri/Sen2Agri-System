#ifndef SLURMSACCTRESULTPARSER_H
#define SLURMSACCTRESULTPARSER_H

#include <QString>
#include "processorexecutioninfos.h"

class SlurmSacctResultParser
{
public:
    SlurmSacctResultParser();
    ~SlurmSacctResultParser();
    int ParseResults(QString &strLog, QList<ProcessorExecutionInfos> &executionInfos, QString *pJobNameFilter = NULL);

    enum {
        JOB_ID_POS = 0,
        JOB_NAME_POS,
        JOB_NODE_POS,
        JOB_CPU_TIME_POS,
        JOB_USER_TIME_POS,
        JOB_SYSTEM_TIME_POS,
        JOB_EXIT_CODE_POS,
        JOB_AVE_VM_SIZE_POS,
        JOB_MAX_RSS_POS,
        JOB_MAX_VM_SIZE_POS,
        JOB_DISK_READ_POS,
        JOB_DISK_WRITE_POS,
        JOB_MAX_POS
    };

private:
    int ParseColumnNames(QString &strColumnsLine);
    bool ParseLine(QString &strLine, ProcessorExecutionInfos &procExecInfos);
    int *m_pPosArr;
    int m_nPosCnt;

    static QString g_strJobId;
    static QString g_strJobName;
    static QString g_strJobNode;
    static QString g_strCpuTime;
    static QString g_strUserTime;
    static QString g_strSystemTime;
    static QString g_strExitCode;
    static QString g_strAveVmSize;
    static QString g_strMaxRss;
    static QString g_strMaxVmSize;
    static QString g_strDiskRead;
    static QString g_strDiskWrite;
};

#endif // SLURMSACCTRESULTPARSER_H
