#ifndef SIMPLEUDPINFOSCLIENT_H
#define SIMPLEUDPINFOSCLIENT_H


#include <QUdpSocket>
#include <QObject>
#include "abstractexecinfosprotclient.h"

class SimpleUdpInfosClient : public AbstractExecInfosProtClient
{
public:
    SimpleUdpInfosClient();
    ~SimpleUdpInfosClient();

    virtual bool Initialize(QString &strIpAddr, int nPortNo);
    virtual bool SendMessage(QString &strMsg);

private:
    QHostAddress m_hostAddr;
    int m_nPortNo;
    QUdpSocket m_udpSocket;
};

#endif // SIMPLEUDPINFOSCLIENT_H
