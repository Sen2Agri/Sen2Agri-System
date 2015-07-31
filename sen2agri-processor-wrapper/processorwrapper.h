#ifndef PROCESSORWRAPPER_H
#define PROCESSORWRAPPER_H

#include <QList>
#include <QString>
#include <QWaitCondition>
#include <QThread>
#include <QMutex>
#include "abstractexecinfosprotclient.h"
#include "icommandinvokerlistener.h"

class ProcessorWrapper : public ICommandInvokerListener
{
public:
    ProcessorWrapper();
    ~ProcessorWrapper();

    bool Initialize(QStringList &listParams);
    virtual void OnNewMessage(QString &strMsg);

    bool ExecuteProcessor();

private:
    QString m_strProcPath;
    QString m_strProcParams;

    QString m_strJobName;

    AbstractExecInfosProtClient *m_pUdpClient;
    QStringList m_listProcParams;
};

#endif // PROCESSORWRAPPER_H
