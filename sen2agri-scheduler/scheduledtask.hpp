#ifndef SCHEDULEDTASK_HPP
#define SCHEDULEDTASK_HPP

#include <QString>
#include <QDateTime>

struct ScheduledTaskStatus
{
    int taskId;

    QDateTime nextScheduledRunTime;

    QDateTime lastSuccesfullScheduledRun; // last succ. scheduleded launch
    QDateTime lastSuccesfullTimestamp; // the moment of last launch
    QDateTime lastRetryTime;

    QDateTime estimatedRunTime; // last succ. scheduleded launch
};

enum RepeatType {
    REPEATTYPE_ONCE = 0,
    REPEATTYPE_CYCLIC = 1,
    REPEATTYPE_ONDATE = 2
};
struct ScheduledTask
{
    int taskId;
    QString	taskName;
    int processorId;
    QString processorParameters;

    int repeatType; /*once, cyclic, on_date*/
    int repeatAfterDays; /* nr of days to cycle the task */
    int repeatOnMonthDay; /* the day of the month to run the task */

    QDateTime firstScheduledRunTime; /* first configured run-time */

    int  retryPeriod; /* minutes or hours ? to retry if the preconditions are not met */

    int taskPriority;
    // startSeason ?

    ScheduledTaskStatus taskStatus;
};


#endif // SCHEDULEDTASK_HPP
