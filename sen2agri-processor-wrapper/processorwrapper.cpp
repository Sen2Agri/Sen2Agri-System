#include "processorwrapper.h"
#include "commandinvoker.h"
#include <QWaitCondition>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include "simpletcpinfosclient.h"

#include <iostream>
using namespace std;

ProcessorWrapper::ProcessorWrapper()
{
    m_pClient = NULL;
}

ProcessorWrapper::~ProcessorWrapper()
{
    if(m_pClient) {
        delete m_pClient;
    }
}

bool ProcessorWrapper::Initialize(QStringList &listParams)
{
    bool bProcParam = false;
    QString strSrvIpAddr;
    QString strPortNo;

    m_listProcParams.clear();
    for (QStringList::iterator it = listParams.begin(); it != listParams.end(); ++it)
    {
        QString &curParam = (*it);
        if(bProcParam)
        {
            // we found PROC_PARAMS so we just add this param to the list of proc params
            m_listProcParams.append(curParam);
        } else {
            int nEqPos = curParam.indexOf("=");
            if(nEqPos > 0)
            {
                QString strKey = curParam.left(nEqPos);
                QString strVal = curParam.right(curParam.size()-nEqPos-1);
                if(strKey == "PROC_PATH") {
                    if(strVal.isNull() || strVal.isEmpty()) {
                        qCritical() << "Error during initialization: No processor execution command was received!";
                        return false;
                    }
                    m_strProcPath = strVal;
                } else if (strKey == "PROC_PARAMS") {
                    if(strVal.size() > 0) {
                        m_listProcParams.append(strVal);
                    }
                    bProcParam = true;
                } else if (strKey == "JOB_NAME") {
                    if(strVal.isNull() || strVal.isEmpty()) {
                        qCritical() << "Error during initialization: No processor job name was received!";
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

    if(!strSrvIpAddr.isNull() && !strSrvIpAddr.isEmpty() &&
            !strPortNo.isNull() && !strPortNo.isEmpty())
    {
        int nPortNo = strPortNo.toInt();
        if(nPortNo > 0) {
            m_pClient = new SimpleTcpInfosClient();
            m_pClient->Initialize(strSrvIpAddr, nPortNo);
        } else {
            qCritical() << "Error during initialization: No valid server port was received!";
            return false;
        }
    } else {
        qCritical() << "Error during initialization: No valid server configuration was received!";
        return false;
    }

    return true;
}

bool ProcessorWrapper::ExecuteProcessor()
{
    CommandInvoker cmdInvoker;
    cmdInvoker.SetListener(this);

    cout << QDir::currentPath().toStdString().c_str() << endl;

    QDateTime dateTime;
    qint64 startTime = dateTime.currentMSecsSinceEpoch();
    // send a message that the execution of the processor is started
    if(m_pClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>,
        //      EXEC_TIME : <duration of the execution of the processor>,
        //      STATUS: <OK/FAIL>
        // }
        QString strJSon = QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\"}").arg(
                    "STARTED", m_strJobName);
        m_pClient->SendMessage(strJSon);
    }

    bool bRet;
    bRet = cmdInvoker.InvokeCommand(m_strProcPath, m_listProcParams, false);

    qint64 endTime = dateTime.currentMSecsSinceEpoch();

    if(m_pClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>,
        //      EXEC_TIME : <duration of the execution of the processor>,
        //      STATUS: <OK/FAIL>
        // }
        QString strJSon = QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\",\"EXEC_TIME\":\"%3\",\"STATUS\":\"%4\"}").arg(
                    "ENDED", m_strJobName, QString::number(endTime-startTime), bRet?"OK":"FAILED");
        m_pClient->SendMessage(strJSon);
    }

    return bRet;
}

void ProcessorWrapper::OnNewMessage(QString &strMsg)
{
    // first, print it to default output (console)
    cout << strMsg.toStdString().c_str() << endl;

    // send it also to a server if available
    if(m_pClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>
        //      LOG : <the log message>
        // }
        QString strJSon = QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\",\"LOG_MSG\":\"%3\"}").arg(
                    "LOG", m_strJobName, strMsg);
        m_pClient->SendMessage(strJSon);
    }
}

