#ifndef ABSTRACTEXECINFOSPROTCLIENT_H
#define ABSTRACTEXECINFOSPROTCLIENT_H


class AbstractExecInfosProtClient
{
public:
    AbstractExecInfosProtClient(){}
    virtual ~AbstractExecInfosProtClient(){}

    virtual bool Initialize(const QString &strIpAddr, int nPortNo) = 0;
    virtual bool SendMessage(const QString &strMsg) = 0;
};

#endif // ABSTRACTEXECINFOSPROTCLIENT_H
