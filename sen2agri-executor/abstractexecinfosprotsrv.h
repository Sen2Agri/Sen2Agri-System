#ifndef ABSTRACTEXECINFOSPROTSRV_H
#define ABSTRACTEXECINFOSPROTSRV_H

#include <QByteArray>

/**
 * @brief The AbstractExecInfosProtSrv class
 * \note
 * This class implements the communication protocol with the processor
 * wrappers. It receives from the wrappers execution information.
 * TODO: we should have a factory for these servers (maybe we'll have several
 * implementations).
 */

class IProcessorWrapperMsgsListener;

class AbstractExecInfosProtSrv
{
public:
    AbstractExecInfosProtSrv();
    ~AbstractExecInfosProtSrv();

    bool StartCommunication(QString &strSrvAddr, int nPort);
    void StopCommunication();
    void SetProcMsgListener(IProcessorWrapperMsgsListener *pListener);

protected:
    virtual int StartServer(QString &strSrvAddr, int nPort) = 0;
    virtual void StopServer() = 0;
    virtual bool HandleNewMessage(const QByteArray &message);

protected:
    IProcessorWrapperMsgsListener *m_pListener;

private:
    bool m_bServerStarted;
};

typedef AbstractExecInfosProtSrv* (*CreateExecInfosProtSrvFn)(void);


#endif // ABSTRACTEXECINFOSPROTSRV_H
