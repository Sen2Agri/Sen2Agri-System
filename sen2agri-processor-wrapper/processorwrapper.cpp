#include "processorwrapper.h"
#include "commandinvoker.h"
#include <QWaitCondition>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include "simpletcpinfosclient.h"

#include <iostream>

#include "logger.hpp"

using namespace std;

ProcessorWrapper::ProcessorWrapper() { m_pClient = NULL; }

ProcessorWrapper::~ProcessorWrapper()
{
    if (m_pClient) {
        delete m_pClient;
    }
}

bool ProcessorWrapper::Initialize(QStringList &listParams)
{
    bool bProcParam = false;
    QString strSrvIpAddr;
    QString strPortNo;

    m_listProcParams.clear();
    for (QStringList::iterator it = listParams.begin(); it != listParams.end(); ++it) {
        QString &curParam = (*it);
        if (bProcParam) {
            // we found PROC_PARAMS so we just add this param to the list of proc params
            m_listProcParams.append(curParam);
        } else {
            int nEqPos = curParam.indexOf("=");
            if (nEqPos > 0) {
                QString strKey = curParam.left(nEqPos);
                QString strVal = curParam.right(curParam.size() - nEqPos - 1);
                if (strKey == "PROC_PATH") {
                    if (strVal.isNull() || strVal.isEmpty()) {
                        Logger::fatal("Error during initialization: No processor execution command "
                                      "was received!");
                        return false;
                    }
                    m_strProcPath = strVal;
                } else if (strKey == "PROC_PARAMS") {
                    if (strVal.size() > 0) {
                        m_listProcParams.append(strVal);
                    }
                    bProcParam = true;
                } else if (strKey == "JOB_NAME") {
                    if (strVal.isNull() || strVal.isEmpty()) {
                        Logger::fatal(
                            "Error during initialization: No processor job name was received!");
                        return false;
                    }
                    m_strJobName = strVal;
                } else if (strKey == "SRV_IP_ADDR") {
                    strSrvIpAddr = strVal;
                } else if (strKey == "SRV_PORT_NO") {
                    strPortNo = strVal;
                }
            } else if (curParam == "PROC_PARAMS") {
                bProcParam = true;
            } // otherwise ignore this parameter
        }
    }

    if (!strSrvIpAddr.isNull() && !strSrvIpAddr.isEmpty() && !strPortNo.isNull() &&
        !strPortNo.isEmpty()) {
        int nPortNo = strPortNo.toInt();
        if (nPortNo > 0) {
            m_pClient = new SimpleTcpInfosClient();
            m_pClient->Initialize(strSrvIpAddr, nPortNo);
        } else {
            Logger::fatal("Error during initialization: No valid server port was received!");
            return false;
        }
    } else {
        Logger::fatal("Error during initialization: No valid server configuration was received!");
        return false;
    }

    return true;
}

QString getJobStartedJson(const QString &jobName)
{
    QJsonObject obj;
    obj["MSG_TYPE"] = "STARTED";
    obj["JOB_NAME"] = jobName;
    return QString::fromUtf8(QJsonDocument(obj).toJson());
}

QString getJobFinishedJson(const QString &jobName,
                           int64_t execTime,
                           const QString &status,
                           int exitCode,
                           const QString &stdOutText,
                           const QString &stdErrText)
{
    QJsonObject obj;
    obj["MSG_TYPE"] = "ENDED";
    obj["JOB_NAME"] = jobName;
    obj["EXEC_TIME"] = QString::number(execTime);
    obj["STATUS"] = status;
    obj["EXIT_CODE"] = QString::number(exitCode);
    obj["STDOUT_TEXT"] = stdOutText;
    obj["STDERR_TEXT"] = stdErrText;
    return QString::fromUtf8(QJsonDocument(obj).toJson());
}

bool ProcessorWrapper::ExecuteProcessor()
{
    CommandInvoker cmdInvoker;
    cmdInvoker.SetListener(this);

    QDateTime dateTime;
    qint64 startTime = dateTime.currentMSecsSinceEpoch();

    Logger::info(QStringLiteral("Job %1 starting").arg(m_strJobName));

    // send a message that the execution of the processor is started
    if (m_pClient) {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>,
        //      EXEC_TIME : <duration of the execution of the processor>,
        //      STATUS: <OK/FAIL>
        // }
        const auto &strJSon = getJobStartedJson(m_strJobName);
        Logger::debug(QStringLiteral("Sending message %1").arg(strJSon));
        m_pClient->SendMessage(strJSon);
    } else {
        Logger::error("No client instance?!");
    }

    bool bRet;
    int exitCode;
    bRet = cmdInvoker.InvokeCommand(m_strProcPath, m_listProcParams, false, exitCode);

    Logger::info(QStringLiteral("Job %1 finished").arg(m_strJobName));

    qint64 endTime = dateTime.currentMSecsSinceEpoch();

    if (m_pClient) {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>,
        //      EXEC_TIME : <duration of the execution of the processor>,
        //      STATUS: <OK/FAIL>
        // }
        const auto &strJson =
            getJobFinishedJson(m_strJobName, endTime - startTime, bRet ? "OK" : "FAILED", exitCode,
                               cmdInvoker.GetStandardOutputLog(), cmdInvoker.GetStandardErrorLog());
        Logger::debug(QStringLiteral("Sending message %1").arg(strJson));
        m_pClient->SendMessage(strJson);
    } else {
        Logger::error("No client instance?!");
    }

    return bRet;
}

void ProcessorWrapper::OnNewMessage(const QString &strMsg)
{
    // first, print it
    Logger::info(strMsg);

    /*
    // send it also to a server if available
    if(m_pClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>
        //      LOG : <the log message>
        // }
        QString strJSon =
    QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\",\"LOG_MSG\":\"%3\"}").arg(
                    "LOG", m_strJobName, strMsg);
        m_pClient->SendMessage(strJSon);
    }
    */
}
