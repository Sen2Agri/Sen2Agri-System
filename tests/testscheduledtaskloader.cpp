#include "testscheduletaskloader.hpp"

TestScheduledTaskLoader::TestScheduledTaskLoader()
{

}
TestScheduledTaskLoader::~TestScheduledTaskLoader()
{

}
std::vector<ScheduledTask> TestScheduledTaskLoader::LoadFromDatabase( )
{
    return m_dbTasks;
}

void TestScheduledTaskLoader::UpdateStatusinDatabase( const std::vector<ScheduledTask>& tasks)
{
    m_dbTasks = tasks;
}

void TestScheduledTaskLoader::setDBTasks(std::vector<ScheduledTask>& tasks)
{
    m_dbTasks = tasks;
}

std::vector<ScheduledTask> TestScheduledTaskLoader::getDBTasks()
{
    return m_dbTasks;
}
