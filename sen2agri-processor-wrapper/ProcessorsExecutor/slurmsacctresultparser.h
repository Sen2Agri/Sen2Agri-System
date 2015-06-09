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
        JOB_NAME_POS = 1,
        JOB_CPU_TIME_POS = 2,
        JOB_AVE_VM_SIZE_POS = 3,
        JOB_MAX_VM_SIZE_POS = 4,
        JOB_MAX_POS = 5
    };

private:
    int ParseColumnNames(QString &strColumnsLine);
    bool ParseLine(QString &strLine, ProcessorExecutionInfos &procExecInfos);
    int *m_pPosArr;
    int m_nPosCnt;

    static QString g_strJobId;
    static QString g_strJobName;
    static QString g_strCpuTime;
    static QString g_strAveVmSize;
    static QString g_strMaxVmSize;
};

#endif // SLURMSACCTRESULTPARSER_H
