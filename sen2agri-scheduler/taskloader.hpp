#ifndef TASKLOADER_H
#define TASKLOADER_H

#include <vector>
#include "scheduledtask.hpp"

class TaskLoader
{
public:
    virtual ~TaskLoader();
    virtual std::vector<ScheduledTask> LoadFromDatabase( ) = 0;
    virtual void UpdateStatusinDatabase( const std::vector<ScheduledTask>& ) = 0;
};

#endif // TASKLOADER_H
