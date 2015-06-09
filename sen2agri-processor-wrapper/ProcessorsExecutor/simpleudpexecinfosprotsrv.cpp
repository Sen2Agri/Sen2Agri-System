#include <QHostAddress>
#include "simpleudpexecinfosprotsrv.h"

SimpleUdpExecInfosProtSrv::SimpleUdpExecInfosProtSrv()
{
    m_pUdpSocket = NULL;
}

SimpleUdpExecInfosProtSrv::~SimpleUdpExecInfosProtSrv()
{
    StopServer();
}

int SimpleUdpExecInfosProtSrv::StartServer(QString &strSrvAddr, int nPort)
{
    m_pUdpSocket = new QUdpSocket(this);
    m_pUdpSocket->bind(QHostAddress(strSrvAddr), nPort);

    connect(m_pUdpSocket, SIGNAL(readyRead()),
        this, SLOT(readPendingDatagrams()));

    return -1;
}

void SimpleUdpExecInfosProtSrv::StopServer()
{
    if(m_pUdpSocket)
        delete m_pUdpSocket;
    m_pUdpSocket = NULL;
}

void SimpleUdpExecInfosProtSrv::readPendingDatagrams()
{
    while (m_pUdpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(m_pUdpSocket->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        m_pUdpSocket->readDatagram(datagram.data(), datagram.size(),
                                &sender, &senderPort);

        HandleNewMessage(datagram);
    }
}

