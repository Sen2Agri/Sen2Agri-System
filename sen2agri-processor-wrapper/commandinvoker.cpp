#include "logger.hpp"

#include "commandinvoker.h"

CommandInvoker::CommandInvoker() { m_pListener = NULL; }

CommandInvoker::~CommandInvoker() {}

void CommandInvoker::SetListener(ICommandInvokerListener *pListener) { m_pListener = pListener; }

bool CommandInvoker::InvokeCommand(QString &strCmd, QStringList &listParams, bool bIsAsync, int &exitCode)
{
    bool bRet = true;
    exitCode = -1;

    m_stdOutText.clear();
    m_stdErrText.clear();

    m_process.start(strCmd, listParams);
    if (m_process.waitForStarted(-1)) {
        if (!bIsAsync) {
            if (!m_process.waitForFinished(-1)) {
                Logger::error(QStringLiteral("Unable to wait for process to finish: %1")
                                  .arg(m_process.errorString()));

                // NOTE: this uses the default timeout.
                // we don't know what happened, but reading the standard output can't hurt
                m_process.waitForReadyRead();
                if(m_process.exitStatus() == QProcess::CrashExit) {
                    bRet = false;
                }
            }
            exitCode = m_process.exitCode();
            if(exitCode != 0) {
                bRet = false;
            }

            const auto &stdOut = m_process.readAllStandardOutput();
            const auto &stdErr = m_process.readAllStandardError();

            m_stdOutText = QString::fromLocal8Bit(stdOut);
            m_stdErrText = QString::fromLocal8Bit(stdErr);

            if (m_pListener) {
                m_pListener->OnNewMessage(m_stdOutText);
                m_pListener->OnNewMessage(m_stdErrText);
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

const QString &CommandInvoker::GetStandardOutputLog() const { return m_stdOutText; }

const QString &CommandInvoker::GetStandardErrorLog() const { return m_stdErrText; }
