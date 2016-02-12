#ifndef TESTSCHEDULEDTASKLOADER_H
#define TESTSCHEDULEDTASKLOADER_H

#include "taskloader.hpp"

class TestScheduledTaskLoader : public TaskLoader
{
public:
    TestScheduledTaskLoader();
    virtual ~TestScheduledTaskLoader();
    // app API
    virtual std::vector<ScheduledTask> LoadFromDatabase( );
    virtual void UpdateStatusinDatabase( std::vector<ScheduledTask>& );

    // test interface
    void setDBTasks(std::vector<ScheduledTask>& tasks);
    std::vector<ScheduledTask> getDBTasks();

private:
    std::vector<ScheduledTask> m_dbTasks;
};

#endif // TESTSCHEDULEDTASKLOADER_H
