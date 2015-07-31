#include "commandinvoker.h"

CommandInvoker::CommandInvoker()
{
    m_pListener = NULL;
}

CommandInvoker::~CommandInvoker()
{

}

void CommandInvoker::SetListener(ICommandInvokerListener *pListener)
{
    m_pListener = pListener;
}

bool CommandInvoker::InvokeCommand(QString &strCmd, bool bIsAsync)
{
    bool bRet = true;
    QString outputStr;

    m_logStr.clear();
    m_process.start(strCmd);
    if (m_process.waitForStarted(-1)) {
        if(!bIsAsync) {
            while(m_process.waitForReadyRead(-1)) {
                QByteArray ba = m_process.readAllStandardOutput();
                outputStr = QString(ba);
                m_logStr += outputStr;
                if(m_pListener)
                    m_pListener->OnNewMessage(outputStr);
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
