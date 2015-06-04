#ifndef ORCHESTRATORREQUESTSHANDLER_H
#define ORCHESTRATORREQUESTSHANDLER_H

#include <QString>
#include <QObject>

/**
 * @brief The OrchestratorRequestsHandler class
 * \note
 * This class is in charge with handling the requests via DBus from the orchestrator
 */
class OrchestratorRequestsHandler : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE bool ExecuteProcessor(const QString &jsonCfgStr);
    Q_INVOKABLE bool StopProcessorJob(const QString &jobName);
};

#endif // ORCHESTRATORREQUESTSHANDLER_H
