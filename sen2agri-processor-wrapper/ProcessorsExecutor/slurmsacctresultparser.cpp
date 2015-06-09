#include <QStringList>

#include "slurmsacctresultparser.h"
#include "processorexecutioninfos.h"

/*static*/
QString SlurmSacctResultParser::g_strJobId("jobid");
/*static*/
QString SlurmSacctResultParser::g_strJobName("jobname");
/*static*/
QString SlurmSacctResultParser::g_strCpuTime("avecpu");
/*static*/
QString SlurmSacctResultParser::g_strAveVmSize("avevmsize");
/*static*/
QString SlurmSacctResultParser::g_strMaxVmSize("maxvmsize");

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

                JobID|JobName|AveCPU|AveVMSize|MaxVMSize
                2|ls|00:00:00|0|0
                3|ls|00:00:00|0|0
                4|find|00:00:00|0|0
                5|hostname|00:00:00|0|0
                6|find|00:00:00|0|0
                7|20150604144546391_CROP_TYPE|00:00:00|0|0
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
    //QRegExp rx2("|"); //RegEx for '|'
    QStringList strColNames = strColumnsLine.split("|");
    m_nPosCnt = strColNames.size();
    if(m_nPosCnt > 0)
    {
        m_pPosArr = new int[m_nPosCnt];
        QListIterator<QString> itr (strColNames);
        int curPos = 0;
        while (itr.hasNext()) {
            QString curColumn = itr.next().toLower();
            if(curColumn == g_strJobId) {
                m_pPosArr[curPos] = JOB_ID_POS;
            } else if(curColumn == g_strJobName) {
                m_pPosArr[curPos] = JOB_NAME_POS;
            } else if(curColumn == g_strCpuTime) {
                m_pPosArr[curPos] = JOB_CPU_TIME_POS;
            } else if(curColumn == g_strAveVmSize) {
                m_pPosArr[curPos] = JOB_AVE_VM_SIZE_POS;
            } else if(curColumn == g_strMaxVmSize) {
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
    //QRegExp rx2("|"); //RegEx for '|'
    QStringList strLineFields = strLine.split("|");
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
