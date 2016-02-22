#include "runestimator.hpp"


RunEstimator::RunEstimator()
{

}

QDateTime RunEstimator::estimateFreeTime(NodeLoad& rn)
{
    Q_UNUSED(rn);
    // TODO:
    //      - the node is already free ? => time = now
    // else
    // - read running tasks from the database per node
    // - read existing time estimation for each processor from the database
    // - add estimation to the start time for each processor => estimated end time for each processor
    // - weight the estimated end time => estimated time when the is free

    return QDateTime::currentDateTime();
}

void RunEstimator::estimateTasks(std::vector<ScheduledTask> tasks)
{
    std::vector<NodeLoad> nodes = ResourceReader().readSystemLoad();
    std::vector<QDateTime> freeTime;

    unsigned i=0;
    for (auto& n : nodes)
    {
        freeTime[i] = estimateFreeTime(n);
        i++;
    }

    // we suppose only 1 processor can be runed at a given time
    for (auto& task : tasks)
    {
        // find next free
        QDateTime quickestFree = QDateTime::currentDateTime().addYears(1);
        unsigned quickestNode=0;
        for (i=0; i<nodes.size(); i++)
        {
            if ( quickestFree < freeTime[i] )
            {
                quickestFree = freeTime[i];
                quickestNode = i;
            }
        }
        // estimate the processor will run on this node
        task.taskStatus.estimatedRunTime = quickestFree;
        freeTime[quickestNode].addSecs(0); // task.averageExecutionTime;
    }
}
