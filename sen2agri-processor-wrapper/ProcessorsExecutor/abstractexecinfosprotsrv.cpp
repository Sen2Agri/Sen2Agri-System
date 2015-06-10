#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>

#include "abstractexecinfosprotsrv.h"
#include "iprocessorwrappermsgslistener.h"
#include "logger.h"

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
        QJsonParseError err;
        QJsonDocument document = QJsonDocument::fromJson(message, &err);
        if(err.error != QJsonParseError::NoError || !document.isObject())
        {
            Logger::GetInstance()->error("An error occurred during parsing message %s", QString(message).toStdString().c_str());
            return false;
        }

        QVariantMap msgVals = document.object().toVariantMap();
        m_pListener->OnProcessorNewMsg(msgVals);
    }

    return true;
}

void AbstractExecInfosProtSrv::SetProcMsgListener(IProcessorWrapperMsgsListener *pListener)
{
    m_pListener = pListener;
}
