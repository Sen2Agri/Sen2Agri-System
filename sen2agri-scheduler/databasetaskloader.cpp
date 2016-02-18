#include <QCoreApplication>

#include "databasetaskloader.hpp"
#include "persistencemanager.hpp"
#include "configuration.hpp"
#include "make_unique.hpp"

DatabaseTaskLoader::DatabaseTaskLoader()
    : persistenceManager(
          Settings::readSettings(getConfigurationFile(*QCoreApplication::instance())))
{
}

DatabaseTaskLoader::~DatabaseTaskLoader()
{
}

std::vector<ScheduledTask> DatabaseTaskLoader::LoadFromDatabase()
{
    return persistenceManager.GetScheduledTasks();
}

void DatabaseTaskLoader::UpdateStatusinDatabase(const std::vector<ScheduledTask> &taskList)
{
    std::vector<ScheduledTaskStatus> statusList;
    for (const auto &t : taskList) {
        statusList.push_back(t.taskStatus);
    }
    persistenceManager.UpdateScheduledTasksStatus(statusList);
}
