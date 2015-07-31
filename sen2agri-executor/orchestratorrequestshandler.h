#ifndef ORCHESTRATORREQUESTSHANDLER_H
#define ORCHESTRATORREQUESTSHANDLER_H

#include <QString>
#include <QObject>

#include "model.hpp"

/**
 * @brief The OrchestratorRequestsHandler class
 * \note
 * This class is in charge with handling the requests via DBus from the orchestrator
 */
class OrchestratorRequestsHandler : public QObject
{
    Q_OBJECT

public:
    Q_INVOKABLE void SubmitSteps(const NewExecutorStepList &steps);
    Q_INVOKABLE void CancelTasks(const TaskIdList &tasks);
};

#endif // ORCHESTRATORREQUESTSHANDLER_H
