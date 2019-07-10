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
        Logger::info("Loading tasks from database ...");
        qDebug() << ("Loading tasks from database ...");
        taskList = m_loader->LoadFromDatabase();
        if(taskList.size() == 0) {
            Logger::info("No tasks defined in the database yet!");
            qDebug() << ("No tasks defined in the database yet!");
            return;
        }

        Logger::info("Computing next run time and updating status in database... ");
        qDebug() << ("Computing next run time and updating status in database... ");
        planner.computeNextRunTime(taskList);
        // save the updated nextScheduleTime to database
        m_loader->UpdateStatusinDatabase(taskList);

        Logger::info("Extracting ready list... ");
        qDebug() << ("Extracting ready list... ");
        std::vector<ScheduledTask> readyList;
        readyList = planner.extractReadyList(taskList);
        planner.orderByPriority(readyList);

        Logger::info("Ready list extracted!");
        qDebug() << ("Ready list extracted!");
        // we'll use a defensive strategy : only one task will be launched in a cycle
        for (auto& task : readyList)
        {
            prequest.processorId = task.processorId;
            prequest.siteId = task.siteId;
            prequest.ttNextScheduledRunTime = (int)task.taskStatus.nextScheduledRunTime.toTime_t();
            prequest.parametersJson = GetTaskParametersJson(task);
            qDebug() << "Getting job definition for processor " << task.processorId << " and site " << task.siteId;
            Logger::info(QStringLiteral("Getting job definition for processor %1 and site %2")
                          .arg(task.processorId).arg(task.siteId));
            jd = m_orchestrator->GetJobDefinition(prequest);
            qDebug() << "Job definition got " << jd.isValid;
            Logger::info(QStringLiteral("Job definition got %1").arg(task.processorId));
            if ( jd.isValid )
            {
                // Optional aproach : only one processor run at a time : KO => done in SLURM
                qDebug() << "Submitting job ...";
                m_orchestrator->SubmitJob(jd);
                task.taskStatus.lastSuccesfullTimestamp = QDateTime::currentDateTime();
                task.taskStatus.lastSuccesfullScheduledRun = task.taskStatus.nextScheduledRunTime;
                Logger::info(QStringLiteral("Submitted new job for processor:  %1, siteId: %2  and definition: %3")
                             .arg(jd.processorId).arg(jd.siteId).arg(jd.jobDefinitionJson.toStdString().c_str()));
                qDebug() << "Submitted new job for processor: " << jd.processorId <<
                            ", siteId: " << jd.siteId <<
                            " and definition: " << jd.jobDefinitionJson.toStdString().c_str();
                // Defensive strategy
                break;
            }
            else
            {
                Logger::info(QStringLiteral("The job for processor: %1, siteId: %2 cannot be started now as is invalid")
                             .arg(jd.processorId).arg(jd.siteId));
                qDebug() << "The job for processor: " << jd.processorId <<
                            ", siteId: " << jd.siteId <<
                            " cannot be started now as is invalid";
                task.taskStatus.lastRetryTime = QDateTime::currentDateTime();
            }            
        }

        // save this changes to database for the tried and launched tasks
        qDebug() << "Updating ready list to database ...";
        Logger::info("Updating ready list to database ...");
        m_loader->UpdateStatusinDatabase(readyList);
        qDebug() << "Updating ready list to database ... Done!";
        Logger::info("Updating ready list to database ... Done!");
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
    QJsonObject cfgParamsObj;
    genParamsObj["task_id"] = QString::number(task.taskId);
    genParamsObj["task_name"] = task.taskName;
    genParamsObj["task_description"] = "";
    genParamsObj["task_type"] = "scheduled";
    genParamsObj["task_repeat_type"] = QString::number(task.repeatType);
    genParamsObj["site_season_id"] = QString::number(task.seasonId);

    const auto &doc = QJsonDocument::fromJson(task.processorParameters.toUtf8());
    if (doc.isObject()) {
        const auto &inObj = doc.object();
        const auto &taskGeneralParamNode = inObj[QStringLiteral("general_params")];
        if (taskGeneralParamNode.isObject()) {
            QJsonObject taskGenParamsObj = taskGeneralParamNode.toObject();
            for(QJsonObject::const_iterator iter = taskGenParamsObj.begin(); iter != taskGenParamsObj.end (); ++iter) {
                genParamsObj[iter.key()] = iter.value().toString();
            }
        }
        const auto cfgParamsNode = inObj[QStringLiteral("config_params")];
        if (cfgParamsNode.isObject()) {
            cfgParamsObj = cfgParamsNode.toObject();
        }
    }

    jsonObj["general_params"] = genParamsObj;
    jsonObj["config_params"] = cfgParamsObj;

    return jsonToString(jsonObj);
}
