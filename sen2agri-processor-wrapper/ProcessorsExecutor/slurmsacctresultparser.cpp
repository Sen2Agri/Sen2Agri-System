#include "slurmsacctresultparser.h"
#include <QStringList>
#include "processorexecutioninfos.h"

SlurmSacctResultParser::SlurmSacctResultParser()
{
    m_pPosArr = NULL;
    m_nPosCnt = 0;
}

SlurmSacctResultParser::~SlurmSacctResultParser()
{

}
int SlurmSacctResultParser::ParseResults(QString &strLog, QList<ProcessorExecutionInfos> &executionInfos, QString *pJobNameFilter)
{
    /* The expected log is similar with the one below:

        Jobid      Jobname    Partition Account    AllocCPUS  State     ExitCode
        ---------- ---------- ---------- ---------- ------- ---------- --------
        3          sja_init   andy       acct1            1 COMPLETED         0
        4          sjaload    andy       acct1            2 COMPLETED         0
        5          sja_scr1   andy       acct1            1 COMPLETED         0
        6          sja_scr2   andy       acct1           18 COMPLETED         2
        7          sja_scr3   andy       acct1           18 COMPLETED         0
        8          sja_scr5   andy       acct1            2 COMPLETED         0
        9          sja_scr7   andy       acct1           90 COMPLETED         1
        10         endscript  andy       acct1          186 COMPLETED         0
    */
    QRegExp rx("(\\n)"); //RegEx for '\n'
    QStringList strList = strLog.split(rx);

    if(strList.size() > 1) {
        // Handle the first line
        QString firstLine = strList.front();
        if(ParseColumnNames(firstLine) > 0) {
            // remove the first line as this is already processed
            strList.removeFirst();

            // now iterate through the lines of the file
            QListIterator<QString> itr2 (strList);
            while (itr2.hasNext()) {
                QString curLine = itr2.next();
                ProcessorExecutionInfos procExecInfos;
                if(ParseLine(curLine, procExecInfos))
                {
                    if(pJobNameFilter) {
                        if(procExecInfos.GetJobName() == *pJobNameFilter) {
                            // add it and exit the loop
                            executionInfos.append(procExecInfos);
                            break;
                        }
                    } else {
                        // just add it
                        executionInfos.append(procExecInfos);
                    }
                }
            }
        }
    }
    return executionInfos.size();
}

int SlurmSacctResultParser::ParseColumnNames(QString &strColumnsLine)
{
    QRegExp rx2("\\s+"); //RegEx for '\\s+'
    QStringList strColNames = strColumnsLine.split(rx2);
    m_nPosCnt = strColNames.size();
    if(m_nPosCnt > 0)
    {
        m_pPosArr = new int[m_nPosCnt];
        QListIterator<QString> itr (strColNames);
        int curPos = 0;
        while (itr.hasNext()) {
            QString curColumn = itr.next().toLower();
            if(curColumn == szJobIdStr) {
                m_pPosArr[curPos] = JOB_ID_POS;
            } else if(curColumn == szJobNameStr) {
                m_pPosArr[curPos] = JOB_NAME_POS;
            } else if(curColumn == szCpuTimeStr) {
                m_pPosArr[curPos] = JOB_CPU_TIME_POS;
            } else if(curColumn == szAveVmSizeStr) {
                m_pPosArr[curPos] = JOB_AVE_VM_SIZE_POS;
            } else if(curColumn == szMaxVmSizeStr) {
                m_pPosArr[curPos] = JOB_MAX_VM_SIZE_POS;
            } else {
                m_pPosArr[curPos] = -1;
            }
            curPos++;
        }
    }

    return m_nPosCnt;
}

bool SlurmSacctResultParser::ParseLine(QString &strLine, ProcessorExecutionInfos &procExecInfos)
{
    QRegExp rx2("\\s+"); //RegEx for '\\s+'
    QStringList strLineFields = strLine.split(rx2);
    QListIterator<QString> itrFields (strLineFields);
    int curPos = 0;
    // check if all fields were filled -> initially set to 0000111111 (or something)
    int nMaskFieldsExtract = (1 << JOB_MAX_POS) - 1;
    while (itrFields.hasNext()) {
        QString curField = itrFields.next();
        if(curPos < m_nPosCnt) {
            int nPosId = m_pPosArr[curPos];
            switch (nPosId) {
                case JOB_ID_POS:
                    procExecInfos.SetJobId(curField);
                    // clear the bit in the mask
                    nMaskFieldsExtract &= ~(1 << JOB_ID_POS);
                    break;
                case JOB_NAME_POS:
                    procExecInfos.SetJobName(curField);
                    nMaskFieldsExtract &= ~(1 << JOB_NAME_POS);
                    break;
                case JOB_CPU_TIME_POS:
                    procExecInfos.SetCpuTime(curField);
                    nMaskFieldsExtract &= ~(1 << JOB_CPU_TIME_POS);
                    break;
                case JOB_AVE_VM_SIZE_POS:
                    procExecInfos.SetAveVmSize(curField);
                    nMaskFieldsExtract &= ~(1 << JOB_AVE_VM_SIZE_POS);
                    break;
                case JOB_MAX_VM_SIZE_POS:
                    procExecInfos.SetMaxVmSize(curField);
                    nMaskFieldsExtract &= ~(1 << JOB_MAX_VM_SIZE_POS);
                    break;
                default:
                    break;
            }
        }
        curPos++;
    }
    // check if all fields were filled
    return (nMaskFieldsExtract == 0);
}
