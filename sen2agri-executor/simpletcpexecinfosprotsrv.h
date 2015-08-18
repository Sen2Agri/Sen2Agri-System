#pragma once

#include <QTcpServer>
#include "abstractexecinfosprotsrv.h"

class SimpleTcpExecInfosProtSrv : public QObject, public AbstractExecInfosProtSrv
{
    Q_OBJECT

public:
    SimpleTcpExecInfosProtSrv();
    ~SimpleTcpExecInfosProtSrv();

    virtual int StartServer(QString &strSrvAddr, int nPort);
    virtual void StopServer();

    static AbstractExecInfosProtSrv * Create() { return new SimpleTcpExecInfosProtSrv(); }

private:
    QTcpServer *m_pServer;

private slots:
    void handleConnection();
    void handleMessage(const QByteArray &data);
};
