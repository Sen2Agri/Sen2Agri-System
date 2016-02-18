#ifndef SCHEDULERAPP_H
#define SCHEDULERAPP_H

#include <QObject>

#include "taskplanner.hpp"
#include "resourcereader.hpp"
#include "dbusorchestratorproxy.hpp"
#include "taskloader.hpp"

class SchedulerTests;

class SchedulerApp : public QObject
{
    Q_OBJECT
    friend class SchedulerTests;

public:
    SchedulerApp(TaskLoader * loader,
                 OrchestratorProxy * orchestrator,
                 QObject *parent = 0);

    virtual ~SchedulerApp();
    void StartRunning();
    void StopRunning();

protected:
    void timerEvent(QTimerEvent *event);
    void RunOnce();

private:
    int m_nTimerId;
    TaskLoader* m_loader;
    OrchestratorProxy* m_orchestrator;
};

#endif // SCHEDULERAPP_H
