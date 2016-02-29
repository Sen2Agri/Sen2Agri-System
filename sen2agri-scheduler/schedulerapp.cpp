#include <QDebug>
#include <qcoreevent.h>
#include "logger.hpp"

#include "schedulerapp.hpp"
#include "taskloader.hpp"
#include "taskplanner.hpp"
#include "resourcereader.hpp"
#include "orchestratorproxy.hpp"
#include "json_conversions.hpp"

SchedulerApp::SchedulerApp(TaskLoader * loader, OrchestratorProxy * orchestrator, QObject *parent)
    : QObject(parent),
      m_nTimerId(0),
      m_loader(loader),
      m_orchestrator(orchestrator)
{
}

SchedulerApp::~SchedulerApp()
{
    StopRunning();
}

void SchedulerApp::StartRunning()
{
    if (m_nTimerId == 0) {
        RunOnce();
        m_nTimerId = startTimer(60000);  // 1-minute timer
    }
}

void SchedulerApp::StopRunning()
{
    killTimer(m_nTimerId);
    m_nTimerId = 0;
}

void SchedulerApp::RunOnce()
{
    TaskPlanner planner;
    ProcessingRequest prequest;
    JobDefinition jd;
    std::vector<ScheduledTask> taskList;

    try
    {
        taskList = m_loader->LoadFromDatabase();
        if(taskList.size() == 0)
            return;

        planner.computeNextRunTime(taskList);
        // save the updated nextScheduleTime to database
        m_loader->UpdateStatusinDatabase(taskList);

        std::vector<ScheduledTask> readyList;
        readyList = planner.extractReadyList(taskList);
        planner.orderByPriority(readyList);

        // we'll use a defensive strategy : only one task will be launched in a cycle
        for (auto& task : readyList)
        {
            prequest.processorId = task.processorId;
            prequest.siteId = task.siteId;
            prequest.ttNextScheduledRunTime = (int)task.taskStatus.nextScheduledRunTime.toTime_t();
            prequest.parametersJson = GetTaskParametersJson(task);
            jd = m_orchestrator->GetJobDefinition(prequest);
            // if the task can be launched according to Orchestrator
            if ( jd.isValid )
            {
                // Optional aproach : only one processor run at a time : KO => done in SLURM
                m_orchestrator->SubmitJob(jd);
                task.taskStatus.lastSuccesfullTimestamp = QDateTime::currentDateTime();
                task.taskStatus.lastSuccesfullScheduledRun = task.taskStatus.nextScheduledRunTime;
                // Defensive strategy
                break;
            }
            else
            {
                task.taskStatus.lastRetryTime = QDateTime::currentDateTime();
            }            
        }

        // save this changes to database for the tried and launched tasks
        m_loader->UpdateStatusinDatabase(readyList);
    }
    catch (std::exception e)
    {
        Logger::error(QStringLiteral("Exception caught during planning tasks: %1").arg(e.what()));
        qCritical() << "Exception caught during planning tasks : " << e.what();
        // What if the exception is from saving into database ?
    }
}

void SchedulerApp::timerEvent(QTimerEvent *event)
{
    ResourceReader rr;

    // Check first the available resources
    if ( rr.areResourcesAvailable() )
    {
        qDebug() << "Timer with ResourcesAvailable : " << event->timerId();
        RunOnce();
    }
    else
    {
        qDebug() << "Timer with NO ResourcesAvailable" << event->timerId();
    }
}

QString SchedulerApp::GetTaskParametersJson(const ScheduledTask &task) {
    QJsonObject jsonObj;
    QJsonObject genParamsObj;
    genParamsObj["task_name"] = task.taskName;
    genParamsObj["task_description"] = "";
    genParamsObj["task_type"] = "scheduled";
    jsonObj["general_params"] = genParamsObj;
    jsonObj["processor_params"] = task.processorParameters;

    return jsonToString(jsonObj);
}
