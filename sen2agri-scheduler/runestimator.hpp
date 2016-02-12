#ifndef RUNESTIMATOR_H
#define RUNESTIMATOR_H

#include <vector>
#include <QDateTime>
#include <scheduledtask.hpp>
#include "resourcereader.hpp"

class RunEstimator
{
public:
    RunEstimator();
    QDateTime estimateFreeTime(NodeLoad& rn);
    void estimateTasks(std::vector<ScheduledTask> tasks);

};

#endif // RUNESTIMATOR_H
