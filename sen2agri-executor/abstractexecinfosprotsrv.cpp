#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>

#include "abstractexecinfosprotsrv.h"
#include "iprocessorwrappermsgslistener.h"
#include "logger.hpp"
#include "requestparamsexecutioninfos.h"

AbstractExecInfosProtSrv::AbstractExecInfosProtSrv()
{
    m_pListener = NULL;
    m_bServerStarted = false;
}

AbstractExecInfosProtSrv::~AbstractExecInfosProtSrv()
{
    m_pListener = NULL;
}

bool AbstractExecInfosProtSrv::StartCommunication(QString &strSrvAddr, int nPort)
{
    if(m_bServerStarted)
    {
        return true;
    }
    m_bServerStarted = StartServer(strSrvAddr, nPort);

    return m_bServerStarted;
}

void AbstractExecInfosProtSrv::StopCommunication()
{
    if(!m_bServerStarted) {
        return;
    }
    StopServer();
    m_bServerStarted = false;
}

bool AbstractExecInfosProtSrv::HandleNewMessage(QByteArray &message)
{
    if(m_pListener)
    {
        RequestParamsExecutionInfos *pReqParams = new RequestParamsExecutionInfos();
        if(!pReqParams->ParseMessage(message))
        {
            Logger::error(QString("An error occurred during parsing message %1").arg(QString(message)));
            return false;
        }

        m_pListener->OnProcessorNewMsg(pReqParams);
    }

    return true;
}

void AbstractExecInfosProtSrv::SetProcMsgListener(IProcessorWrapperMsgsListener *pListener)
{
    m_pListener = pListener;
}
