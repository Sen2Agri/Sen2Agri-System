#include "commandinvoker.h"

CommandInvoker::CommandInvoker()
{
}

CommandInvoker::~CommandInvoker()
{

}

bool CommandInvoker::InvokeCommand(QString &strCmd, QStringList &listParams, bool bIsAsync)
{
    m_logStr.clear();

    if (bIsAsync) {
        return QProcess::startDetached(strCmd, listParams);
    }

    bool bRet = true;

    m_process.setWorkingDirectory("/tmp/");

    m_process.start(strCmd, listParams);
    if (m_process.waitForStarted(-1)) {
            while(m_process.waitForReadyRead(-1)) {
                m_logStr += m_process.readAllStandardOutput();
            }
    } else {
        bRet = false;
    }

    return bRet;
}

void CommandInvoker::StopCurCmdExec()
{
    QProcess::ProcessState procState = m_process.state();
    if(procState != QProcess::NotRunning) {
        m_process.kill();
    }
}

QString& CommandInvoker::GetExecutionLog()
{
    return m_logStr;
}
