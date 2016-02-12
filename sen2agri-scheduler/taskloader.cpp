#include "taskloader.hpp"

TaskLoader::TaskLoader()
{

}

TaskLoader::~TaskLoader()
{

}

std::vector<ScheduledTask> TaskLoader::LoadFromDatabase( )
{
    std::vector<ScheduledTask> taskList;

    // TODO: call Persistance mng
    return taskList;
}

void TaskLoader::UpdateStatusinDatabase( std::vector<ScheduledTask>& )
{
    // TODO: call Persistance mng
}

