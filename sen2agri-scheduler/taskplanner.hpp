#ifndef TASKPLANNER_H
#define TASKPLANNER_H

#include <vector>
#include "scheduledtask.hpp"

class TaskPlanner
{
public:
    TaskPlanner();
    void computeNextRunTime(std::vector<ScheduledTask>& tasks);
    std::vector<ScheduledTask> extractReadyList(std::vector<ScheduledTask>& tasks);
    void orderByPriority(std::vector<ScheduledTask>& tasks);

};

#endif // TASKPLANNER_H
