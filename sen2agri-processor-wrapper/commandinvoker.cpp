#include "logger.hpp"

#include "commandinvoker.h"

CommandInvoker::CommandInvoker() { m_pListener = NULL; }

CommandInvoker::~CommandInvoker() {}

void CommandInvoker::SetListener(ICommandInvokerListener *pListener) { m_pListener = pListener; }

bool CommandInvoker::InvokeCommand(QString &strCmd, QStringList &listParams, bool bIsAsync)
{
    bool bRet = true;
    QString outputStr;

    m_logStr.clear();
    if (listParams.isEmpty()) {
        m_process.start(strCmd);
    } else {
        m_process.start(strCmd, listParams);
    }
    if (m_process.waitForStarted(-1)) {
        if (!bIsAsync) {
            if (!m_process.waitForFinished(-1)) {
                Logger::error(QStringLiteral("Unable to wait for process to finish: %1")
                                  .arg(m_process.errorString()));
            }

            while (m_process.waitForReadyRead(-1)) {
                QByteArray ba = m_process.readAllStandardOutput();
                outputStr = QString(ba);
                m_logStr += outputStr;
                if (m_pListener)
                    m_pListener->OnNewMessage(outputStr);
            }
        }
    } else {
        Logger::error(QStringLiteral("Unable to start process: %1").arg(m_process.errorString()));
        bRet = false;
    }

    return bRet;
}

void CommandInvoker::StopCurCmdExec()
{
    QProcess::ProcessState procState = m_process.state();
    if (procState != QProcess::NotRunning) {
        m_process.kill();
    }
}

QString &CommandInvoker::GetExecutionLog() { return m_logStr; }
