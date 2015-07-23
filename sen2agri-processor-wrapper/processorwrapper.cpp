#include "processorwrapper.h"
#include "commandinvoker.h"
#include <QWaitCondition>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include "simpleudpinfosclient.h"

#include <iostream>
using namespace std;

ProcessorWrapper::ProcessorWrapper()
{
    m_pUdpClient = NULL;
}

ProcessorWrapper::~ProcessorWrapper()
{
    if(m_pUdpClient) {
        delete m_pUdpClient;
    }
}

bool ProcessorWrapper::Initialize(QVariantMap &params)
{
    m_strProcPath = params["PROC_PATH"].toString();
    if(m_strProcPath.isNull() || m_strProcPath.isEmpty()) {
        qCritical() << "Error during initialization: No processor execution command was received!";
        return false;
    }
    m_strProcParams = params["PROC_PARAMS"].toString();
    m_strJobName = params["JOB_NAME"].toString();

    QString strSrvIpAddr = params["SRV_IP_ADDR"].toString();
    int nPortNo = params["SRV_PORT_NO"].toInt();

    if(!strSrvIpAddr.isNull() && !strSrvIpAddr.isEmpty())
    {
        m_pUdpClient = new SimpleUdpInfosClient();
        m_pUdpClient->Initialize(strSrvIpAddr, nPortNo);
    }

    return true;
}

bool ProcessorWrapper::ExecuteProcessor()
{
    CommandInvoker cmdInvoker;
    cmdInvoker.SetListener(this);

    cout << QDir::currentPath().toStdString().c_str() << endl;

    QString strCmd = QString("\"%1\" \"%2\"").arg(m_strProcPath, m_strProcParams);
    QDateTime dateTime;
    qint64 startTime = dateTime.currentMSecsSinceEpoch();
    // send a message that the execution of the processor is started
    if(m_pUdpClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>,
        //      EXEC_TIME : <duration of the execution of the processor>,
        //      STATUS: <OK/FAIL>
        // }
        QString strJSon = QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\"}").arg(
                    "STARTED", m_strJobName);
        m_pUdpClient->SendMessage(strJSon);
    }

    bool bRet = cmdInvoker.InvokeCommand(strCmd, false);
    qint64 endTime = dateTime.currentMSecsSinceEpoch();

    if(m_pUdpClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>,
        //      EXEC_TIME : <duration of the execution of the processor>,
        //      STATUS: <OK/FAIL>
        // }
        QString strJSon = QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\",\"EXEC_TIME\":\"%3\",\"STATUS\":\"%4\"}").arg(
                    "ENDED", m_strJobName, QString::number(endTime-startTime), bRet?"OK":"FAILED");
        m_pUdpClient->SendMessage(strJSon);
    }

    return bRet;
}

void ProcessorWrapper::OnNewMessage(QString &strMsg)
{
    // first, print it to default output (console)
    cout << strMsg.toStdString().c_str() << endl;

    // send it also to a server if available
    if(m_pUdpClient)
    {
        // A json message will be sent with the following format:
        // {
        //      JOB_NAME : <name of the slurm job as originally created>
        //      LOG : <the log message>
        // }
        QString strJSon = QString("{\"MSG_TYPE\":\"%1\",\"JOB_NAME\":\"%2\",\"LOG_MSG\":\"%3\"}").arg(
                    "LOG", m_strJobName, strMsg);
        m_pUdpClient->SendMessage(strJSon);
    }
}

