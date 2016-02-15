#include "taskloader.hpp"
#include "persistencemanager.hpp"
#include "qcoreapplication.h"
#include "configuration.hpp"

static  PersistenceManagerDBProvider persistenceManager(
        Settings::readSettings(getConfigurationFile(*QCoreApplication::instance())));

TaskLoader::TaskLoader()
{

}

TaskLoader::~TaskLoader()
{

}

std::vector<ScheduledTask> TaskLoader::LoadFromDatabase( )
{
    std::vector<ScheduledTask> taskList;
    taskList = persistenceManager.GetScheduledTasks();
    return taskList;
}

void TaskLoader::UpdateStatusinDatabase( std::vector<ScheduledTask>& taskList)
{
    std::vector<ScheduledTaskStatus> statusList;
    for (auto t: taskList)
    {
        statusList.push_back(t.taskStatus);
    }
    persistenceManager.UpdateScheduledTasksStatus(statusList);
}

