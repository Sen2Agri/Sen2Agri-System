#ifndef PERSISTENCEITFMODULE_H
#define PERSISTENCEITFMODULE_H

#include "persistencemanager.hpp"
#include "processorexecutioninfos.h"

/**
 * @brief The PersistenceItfModule class
 * \note
 * This class represents the interface with the persistence manager
 * application. The communication is made via DBus.
 *
 */

class PersistenceItfModule : public QObject
{
    Q_OBJECT

public:
    ~PersistenceItfModule();

    static PersistenceItfModule *GetInstance();

    void SendProcessorExecInfos(ProcessorExecutionInfos &execInfos);
    void RequestConfiguration();

    void MarkStepPendingStart(int taskId, QString &name);
    void MarkStepStarted(int taskId, QString &name);
    bool MarkStepFinished(int taskId, QString &name, ProcessorExecutionInfos &statistics);
    void MarkStepFailed(int taskId, QString &name, ProcessorExecutionInfos &statistics);

    QString GetExecutorQos(int processorId);
    QString GetExecutorPartition(int processorId);

signals:
    void OnConfigurationReceived();

private:
    PersistenceItfModule();
    PersistenceManagerDBProvider clientInterface;

    bool GetValueForKey(const ConfigurationParameterValueList &configuration,
                        const QString &key, QString &value);
    void SaveMainConfigKeys(const ConfigurationParameterValueList &configuration);
    long ParseTimeStr(const QString &strTime);
    ExecutionStatistics InitStatistics(const ProcessorExecutionInfos &statistics);
};

#endif // PERSISTENCEITFMODULE_H
