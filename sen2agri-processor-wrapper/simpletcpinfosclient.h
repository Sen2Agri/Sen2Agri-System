#pragma once

#include <QString>
#include <QHostAddress>
#include <QTcpSocket>

#include "abstractexecinfosprotclient.h"

class SimpleTcpInfosClient : public AbstractExecInfosProtClient
{
    QString m_hostAddrStr;
    int m_nPortNo;
    QTcpSocket m_socket;

public:
    bool Initialize(const QString &strIpAddr, int nPortNo);
    bool SendMessage(const QString &strMsg);

private:
    bool SendMessage(const QString &strHostAddr, const QString &strMsg);
};
