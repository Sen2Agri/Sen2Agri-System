#include "abstractexecinfosprotsrv.h"

#include <QVariantMap>
#include "QJson/Parser"
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
    QJson::Parser parser;
    bool ok;

    QVariantMap msgVals = parser.parse (message, &ok).toMap();
    if (!ok) {
        Logger::GetInstance()->error("An error occurred during parsing message %s", QString(message).toStdString().c_str());
        return false;
    }

    if(m_pListener)
    {
        m_pListener->OnProcessorNewMsg(msgVals);
    }

    return true;
}

void AbstractExecInfosProtSrv::SetProcMsgListener(IProcessorWrapperMsgsListener *pListener)
{
    m_pListener = pListener;
}
