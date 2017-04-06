#ifndef ABSTRACTEXECINFOSPROTCLIENT_H
#define ABSTRACTEXECINFOSPROTCLIENT_H


class AbstractExecInfosProtClient
{
public:
    AbstractExecInfosProtClient(){
        // one hour retries by default
        m_nSendRetriesNo = 3600;
        m_nTimeoutBetweenRetries = 1000;
        // By default, is assumed the processor executes locally
        m_nExecutesLocal = 1;
    }
    virtual ~AbstractExecInfosProtClient(){}

    virtual bool Initialize(const QString &strIpAddr, int nPortNo) = 0;
    virtual bool SendMessage(const QString &strMsg) = 0;
    virtual void SetRetryParameters(const QString &strSendRetriesNo, const QString &strTimeoutBetweenRetries)
    {
        bool res = false;
        m_nSendRetriesNo = strSendRetriesNo.toInt(&res);
        if (!res) {
            m_nSendRetriesNo = 3600;
        }
        m_nTimeoutBetweenRetries = strTimeoutBetweenRetries.toInt(&res);
        if (!res) {
            m_nTimeoutBetweenRetries = 1000;
        }
    }
    virtual void SetExecutesLocallyFlag(const QString &strExecutesLocal)
    {
        bool res = false;
        m_nExecutesLocal = strExecutesLocal.toInt(&res);
        if (!res) {
            m_nExecutesLocal = 1;
        }
    }

protected:
    int m_nSendRetriesNo;
    int m_nTimeoutBetweenRetries;
    int m_nExecutesLocal;
};

#endif // ABSTRACTEXECINFOSPROTCLIENT_H
