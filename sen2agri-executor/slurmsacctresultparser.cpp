#include <QStringList>
#include <QVector>

#include "slurmsacctresultparser.h"
#include "processorexecutioninfos.h"

/*static*/
QString SlurmSacctResultParser::g_strJobId("jobid");
/*static*/
QString SlurmSacctResultParser::g_strJobName("jobname");
/*static*/
QString SlurmSacctResultParser::g_strJobNode("nodelist");
/*static*/
QString SlurmSacctResultParser::g_strCpuTime("avecpu");
/*static*/
QString SlurmSacctResultParser::g_strUserTime("usercpu");
/*static*/
QString SlurmSacctResultParser::g_strSystemTime("systemcpu");
/*static*/
QString SlurmSacctResultParser::g_strExitCode("exitcode");
/*static*/
QString SlurmSacctResultParser::g_strAveVmSize("avevmsize");
/*static*/
QString SlurmSacctResultParser::g_strMaxVmSize("maxvmsize");
/*static*/
QString SlurmSacctResultParser::g_strMaxRss("maxrss");
/*static*/
QString SlurmSacctResultParser::g_strDiskRead("maxdiskread");
/*static*/
QString SlurmSacctResultParser::g_strDiskWrite("maxdiskwrite");

SlurmSacctResultParser::SlurmSacctResultParser()
{
    m_pPosArr = NULL;
    m_nPosCnt = 0;
}

SlurmSacctResultParser::~SlurmSacctResultParser()
{
    if(m_pPosArr)
    {
        delete[] m_pPosArr;
    }
    m_pPosArr = NULL;
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
                        if(procExecInfos.strJobName == *pJobNameFilter) {
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
        if(m_pPosArr)
            delete[] m_pPosArr;
        m_pPosArr = new int[m_nPosCnt];
        QListIterator<QString> itr (strColNames);
        int curPos = 0;
        while (itr.hasNext()) {
            QString curColumn = itr.next().toLower();
            if(curColumn == g_strJobId) {
                m_pPosArr[curPos] = JOB_ID_POS;
            } else if(curColumn == g_strJobName) {
                m_pPosArr[curPos] = JOB_NAME_POS;
            } else if(curColumn == g_strJobNode) {
                m_pPosArr[curPos] = JOB_NODE_POS;
            } else if(curColumn == g_strCpuTime) {
                m_pPosArr[curPos] = JOB_CPU_TIME_POS;
            } else if(curColumn == g_strUserTime) {
                m_pPosArr[curPos] = JOB_USER_TIME_POS;
            } else if(curColumn == g_strSystemTime) {
                m_pPosArr[curPos] = JOB_SYSTEM_TIME_POS;
            } else if(curColumn == g_strExitCode) {
                m_pPosArr[curPos] = JOB_EXIT_CODE_POS;
            } else if(curColumn == g_strAveVmSize) {
                m_pPosArr[curPos] = JOB_AVE_VM_SIZE_POS;
            } else if(curColumn == g_strMaxRss) {
                m_pPosArr[curPos] = JOB_MAX_RSS_POS;
            } else if(curColumn == g_strMaxVmSize) {
                m_pPosArr[curPos] = JOB_MAX_VM_SIZE_POS;
            } else if(curColumn == g_strDiskRead) {
                m_pPosArr[curPos] = JOB_DISK_READ_POS;
            } else if(curColumn == g_strDiskWrite) {
                m_pPosArr[curPos] = JOB_DISK_WRITE_POS;
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
                    procExecInfos.strJobId = curField;
                    // clear the bit in the mask
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_NAME_POS:
                    procExecInfos.strJobName = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_NODE_POS:
                    procExecInfos.strJobNode = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_CPU_TIME_POS:
                    procExecInfos.strCpuTime = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_USER_TIME_POS:
                    procExecInfos.strUserTime = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_SYSTEM_TIME_POS:
                    procExecInfos.strSystemTime = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_EXIT_CODE_POS:
                    procExecInfos.strExitCode = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_AVE_VM_SIZE_POS:
                    procExecInfos.strAveVmSize = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_MAX_RSS_POS:
                    procExecInfos.strMaxRss = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_MAX_VM_SIZE_POS:
                    procExecInfos.strMaxVmSize = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_DISK_READ_POS:
                    procExecInfos.strDiskRead = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
                    break;
                case JOB_DISK_WRITE_POS:
                    procExecInfos.strDiskWrite = curField;
                    nMaskFieldsExtract &= ~(1 << nPosId);
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
