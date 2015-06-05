#ifndef SLURMSACCTRESULTPARSER_H
#define SLURMSACCTRESULTPARSER_H


#include <QString>
#include "processorexecutioninfos.h"


enum {
    JOB_ID_POS = 0,
    JOB_NAME_POS = 1,
    JOB_CPU_TIME_POS = 2,
    JOB_AVE_VM_SIZE_POS = 3,
    JOB_MAX_VM_SIZE_POS = 4,
    JOB_MAX_POS = 5
};

const char szJobIdStr[] = "jobid";
const char szJobNameStr[] = "jobname";
const char szCpuTimeStr[] = "avecpu";
const char szAveVmSizeStr[] = "avevmsize";
const char szMaxVmSizeStr[] = "maxvmsize";

class SlurmSacctResultParser
{
public:
    SlurmSacctResultParser();
    ~SlurmSacctResultParser();
    int ParseResults(QString &strLog, QList<ProcessorExecutionInfos> &executionInfos, QString *pJobNameFilter = NULL);

private:
    int ParseColumnNames(QString &strColumnsLine);
    bool ParseLine(QString &strLine, ProcessorExecutionInfos &procExecInfos);
    int *m_pPosArr;
    int m_nPosCnt;
};

#endif // SLURMSACCTRESULTPARSER_H
