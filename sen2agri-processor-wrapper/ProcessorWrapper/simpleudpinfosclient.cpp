#include "simpleudpinfosclient.h"

SimpleUdpInfosClient::SimpleUdpInfosClient()
{
}

SimpleUdpInfosClient::~SimpleUdpInfosClient()
{
}

bool SimpleUdpInfosClient::Initialize(QString &strIpAddr, int nPortNo)
{
    m_hostAddr = QHostAddress(strIpAddr);
    m_nPortNo = nPortNo;
    if(m_hostAddr.isNull()) {
        qDebug() << "Invalid IP address " << strIpAddr << ". The messages will not be sent to the server!";
        return false;
    }
    return true;
}

bool SimpleUdpInfosClient::SendMessage(QString &strMsg)
{
    if(!m_hostAddr.isNull())
    {
        QByteArray ba = strMsg.toLatin1();
        return (m_udpSocket.writeDatagram(ba, m_hostAddr, m_nPortNo) == ba.size());
    }

    return false;
}
