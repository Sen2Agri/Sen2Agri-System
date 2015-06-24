#ifndef RESSOURCEMANAGERITF_H
#define RESSOURCEMANAGERITF_H

#include <QThread>
#include <QList>
#include <QMutex>
#include <QWaitCondition>

#include <string>
using namespace std;

#include "iprocessorwrappermsgslistener.h"

enum {PROCESSOR_ENDED = 1, PROCESSOR_INFO_MSG = 2, START_PROCESSOR_REQ = 3, STOP_PROCESSOR_REQ = 4};


/**
 * @brief The RessourceManagerItf class
 * \note
 * This class represents the interface with the Ressource Manager (SLURM).
 * It takes a wraper and executes the SLURM commands like srun, SSTAT, SACCT etc.)
 */
class RessourceManagerItf : public QThread, public IProcessorWrapperMsgsListener
{
    Q_OBJECT

public:
    ~RessourceManagerItf();

    static RessourceManagerItf *GetInstance();
    bool Start();
    void Stop();

    bool StartProcessor(QVariantMap &reqParams);
    bool StopProcessor(QVariantMap &strJobName);

    virtual void OnProcessorNewMsg(QVariantMap &msgVals);

private:
    RessourceManagerItf();
    void run();
    bool m_bStop;
    bool m_bStarted;

    QList<QVariantMap> m_msgQueue;
    QMutex m_syncMutex;
    QWaitCondition m_condition;

    void AddRequestToQueue(QVariantMap &req);

    bool HandleStartProcessor(QVariantMap &reqParams);
    void HandleStopProcessor(QVariantMap &strJobName);

    bool HandleProcessorEndedMsg(QVariantMap &msgVals);
    bool HandleProcessorInfosMsg(QVariantMap &msgVals);
};

#endif // RESSOURCEMANAGERITF_H
