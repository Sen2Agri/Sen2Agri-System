#include "taskplanner.hpp"

TaskPlanner::TaskPlanner()
{

}

void TaskPlanner::computeNextRunTime(std::vector<ScheduledTask>& tasks)
{
    for (auto& task : tasks)
    {
        // TODO: consider other repeat types
        // TODO2: add constant values
        // if the task was executed on its previous cycle
        QDateTime zeroDate; zeroDate.setTime_t(0);
        if ( task.taskStatus.nextScheduledRunTime <= zeroDate )
        {
            task.taskStatus.nextScheduledRunTime = task.firstScheduledRunTime;
            task.taskStatus.estimatedRunTime.setTime_t(0);
            task.taskStatus.lastRetryTime.setTime_t(0);
            task.taskStatus.lastSuccesfullScheduledRun.setTime_t(0);
            task.taskStatus.lastSuccesfullTimestamp.setTime_t(0);
        }

        // runonce task
        if ( task.repeatType == REPEATTYPE_ONCE &&
             task.taskStatus.nextScheduledRunTime <= task.taskStatus.lastSuccesfullScheduledRun )
        {
            // if executed already dont schedule it again
            task.taskStatus.nextScheduledRunTime = task.taskStatus.nextScheduledRunTime.addYears(100);
        }
        // Cyclic task
        //if ( task.repeatType == 1 && task.taskStatus.nextScheduledRunTime.addDays(task.repeatAfterDays)
        //     < task.taskStatus.lastSuccesfullRunTime )
        if ( task.repeatType == REPEATTYPE_CYCLIC &&
             task.taskStatus.nextScheduledRunTime <= task.taskStatus.lastSuccesfullScheduledRun )
        {
            task.taskStatus.nextScheduledRunTime = task.taskStatus.nextScheduledRunTime.addDays(task.repeatAfterDays);
            // reset the estimated time
            task.taskStatus.estimatedRunTime.setTime_t(0);
        }
        // On date task
        if ( task.repeatType == REPEATTYPE_ONDATE &&
             task.taskStatus.nextScheduledRunTime <= task.taskStatus.lastSuccesfullScheduledRun )
             //task.taskStatus.lastSuccesfullRunTime.date() != dateNow.date() &&
             //task.repeatOnMonthDay == dateNow.date().day() &&
             //task.taskStatus.nextScheduledRunTime.date() != dateNow.date() )
        {
            task.taskStatus.nextScheduledRunTime = task.taskStatus.nextScheduledRunTime.addMonths(1);
            // if we have the dates 30,31 the above function makes some corrections if missing
            // revert to setup date to not perpetuate
            QDate dn1;
            dn1.setDate(task.taskStatus.nextScheduledRunTime.date().year(),
                        task.taskStatus.nextScheduledRunTime.date().month(),
                        1);
            // protection to obtaining invalid date when we have the day
            // greater than the days in month
            int monthDayToUse = dn1.daysInMonth();
            if(task.repeatOnMonthDay <= monthDayToUse) {
                monthDayToUse = task.repeatOnMonthDay;
            }
            dn1.setDate(task.taskStatus.nextScheduledRunTime.date().year(),
                        task.taskStatus.nextScheduledRunTime.date().month(),
                        monthDayToUse);
            task.taskStatus.nextScheduledRunTime.setDate(dn1);
            // reset the estimated time
            task.taskStatus.estimatedRunTime.setTime_t(0);
        }
    }
}

std::vector<ScheduledTask> TaskPlanner::extractReadyList(std::vector<ScheduledTask>& tasks)
{
    std::vector<ScheduledTask> readyTasks;
    QDateTime dateNow = QDateTime::currentDateTime();
    // extract when next Schedule Ready and/or when Retry is ready
    for (auto& task : tasks)
    {
        QDateTime nextRetry = task.taskStatus.lastRetryTime.addSecs(task.retryPeriod);
        if ( (task.taskStatus.nextScheduledRunTime <= dateNow) &&
             ( nextRetry <= dateNow) )
        {
            readyTasks.push_back(task);
        }
    }

    return readyTasks;
}

bool lessPriority( const ScheduledTask & e1, const ScheduledTask & e2 )
{
    return e1.taskPriority < e2.taskPriority;
}
void TaskPlanner::orderByPriority(std::vector<ScheduledTask>& tasks)
{
    // lower value is higher priority
    qSort(tasks.begin(), tasks.end(), lessPriority);
}


