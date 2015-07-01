#ifndef APPLICATIONCLOSINGLISTENER_H
#define APPLICATIONCLOSINGLISTENER_H

#include <QObject>

#include "processorsexecutor_interface.h"
#include "persistencemanager.h"

class ApplicationClosingListener : public QObject
{
    Q_OBJECT
public:
    ApplicationClosingListener(QObject *parent = 0);

    void SetPersistenceManager(PersistenceManager *pPersistenceMng)
    {
        m_pPersistenceMng = pPersistenceMng;
    }

public slots:
    void run()
    {
        // Do processing here
        emit finished();
    }
    void SendExecuteProcessor()
    {
        HandleSendExecuteProcessor();
    }

    void SendCancelProcessor()
    {
        HandleSendCancelProcessor();
    }

signals:
    void finished();

private:
    void HandleSendExecuteProcessor();
    void HandleSendCancelProcessor();
    org::esa::sen2agri::processorsExecutor *m_pProcessorExecutor;
    PersistenceManager *m_pPersistenceMng;
};

#endif // APPLICATIONCLOSINGLISTENER_H
