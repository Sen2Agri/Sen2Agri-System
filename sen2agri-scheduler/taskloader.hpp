#ifndef TASKLOADER_H
#define TASKLOADER_H

#include <vector>
#include "scheduledtask.hpp"

class TaskLoader
{
public:
    TaskLoader();
    virtual ~TaskLoader();
    virtual std::vector<ScheduledTask> LoadFromDatabase( );
    virtual void UpdateStatusinDatabase( std::vector<ScheduledTask>& );
};

#endif // TASKLOADER_H
