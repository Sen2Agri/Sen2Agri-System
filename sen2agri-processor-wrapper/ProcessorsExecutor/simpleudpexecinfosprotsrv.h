#ifndef SIMPLEUDPEXECINFOSPROTSRV_H
#define SIMPLEUDPEXECINFOSPROTSRV_H

#include "abstractexecinfosprotsrv.h"
#include <QUdpSocket>

class SimpleUdpExecInfosProtSrv : public QObject, public AbstractExecInfosProtSrv
{
    Q_OBJECT

public:
    SimpleUdpExecInfosProtSrv();
    ~SimpleUdpExecInfosProtSrv();

    virtual int StartServer(QString &strSrvAddr, int nPort);
    virtual void StopServer();

    static AbstractExecInfosProtSrv * Create() { return new SimpleUdpExecInfosProtSrv(); }

private:
    QUdpSocket *m_pUdpSocket;

public slots:
    void readPendingDatagrams();
};

#endif // SIMPLEUDPEXECINFOSPROTSRV_H
