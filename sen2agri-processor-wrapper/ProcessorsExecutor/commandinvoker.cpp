#include "commandinvoker.h"

CommandInvoker::CommandInvoker()
{
}

CommandInvoker::~CommandInvoker()
{

}

bool CommandInvoker::InvokeCommand(QString &strCmd, bool bIsAsync)
{
    bool bRet = true;

    m_logStr.clear();
    m_process.start(strCmd);
    if (m_process.waitForStarted(-1)) {
        if(!bIsAsync) {
            while(m_process.waitForReadyRead(-1)) {
                m_logStr += m_process.readAllStandardOutput();
            }
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
