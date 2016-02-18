#include <QTest>
#include <QList>

#include "schedulertests.h"
#include "reflector.hpp"
#include "reflector_adaptor.h"
#include "schedulerapp.hpp"
#include "testscheduletaskloader.hpp"
#include "testorcherstratorproxy.h"


SchedulerTests::SchedulerTests(QObject *parent) : QObject(parent)
{
}

void SchedulerTests::initTestCase()
{
}

void SchedulerTests::cleanupTestCase()
{
}

void SchedulerTests::testNoReadyTasks()
{
    TestOrcherstratorProxy orchestrator;
    TestScheduledTaskLoader loader;
    SchedulerApp sapp(0, &loader, &orchestrator);
    ScheduledTask st;
    std::vector<ScheduledTask> dbTasks;

    // Task 1
    st.processorId = 1; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONDATE;
    st.repeatOnMonthDay = QDateTime::currentDateTime().addDays(1).date().day();
    st.firstScheduledRunTime = QDateTime::currentDateTime().addDays(1);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    // Task 2
    st.processorId = 2; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_CYCLIC; st.repeatAfterDays = 2;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(100);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    loader.setDBTasks(dbTasks);
    sapp.RunOnce();

    QVERIFY(orchestrator.getSubmitedJobs().empty());

    sapp.RunOnce();
    QVERIFY(orchestrator.getSubmitedJobs().empty());

    // Task 3
    st.processorId = 3; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONCE; st.repeatAfterDays = 0;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(-100);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    // task could be runned, but Orchestrator will not agree
    loader.setDBTasks(dbTasks);
    orchestrator.setNextJobsValid(false);
    sapp.RunOnce();
    QVERIFY(orchestrator.getSubmitedJobs().empty());
}

void SchedulerTests::testFirstReadyTask()
{
    TestOrcherstratorProxy orchestrator;
    TestScheduledTaskLoader loader;
    SchedulerApp sapp(0, &loader, &orchestrator);
    ScheduledTask st;
    std::vector<ScheduledTask> dbTasks;

    // Task 1
    st.processorId = 1; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONDATE;
    st.repeatOnMonthDay = QDateTime::currentDateTime().date().day();
    st.retryPeriod = 1;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addDays(-2);
    dbTasks.push_back(st);

    // Task 2
    st.processorId = 2; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_CYCLIC; st.repeatAfterDays = 1;
    st.retryPeriod = 1;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(-100);
    dbTasks.push_back(st);

    // Task 3
    st.processorId = 3; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONCE; st.repeatAfterDays = 0;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(1000);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    loader.setDBTasks(dbTasks);
    sapp.RunOnce();
    // first task should be executed first
    QVERIFY(orchestrator.getSubmitedJobs().size() == 1);
    QCOMPARE(orchestrator.getSubmitedJobs()[0].processorId, 1);

    sapp.RunOnce();
    // second task should be executed after
    QVERIFY(orchestrator.getSubmitedJobs().size() == 2);
    QCOMPARE(orchestrator.getSubmitedJobs()[1].processorId, 2);

    sapp.RunOnce();
    // third task should be not executed
    QVERIFY(orchestrator.getSubmitedJobs().size() == 2);
}


void SchedulerTests::testSecondRetryTask()
{
    TestOrcherstratorProxy orchestrator;
    TestScheduledTaskLoader loader;
    SchedulerApp sapp(0, &loader, &orchestrator);
    ScheduledTask st;
    std::vector<ScheduledTask> dbTasks;

    // Task 1
    st.processorId = 1; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONDATE;
    st.repeatOnMonthDay = QDateTime::currentDateTime().date().day();
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(100);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    // Task 2
    st.processorId = 2; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_CYCLIC; st.repeatAfterDays = 1;
    st.retryPeriod = 0;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(-100);
    dbTasks.push_back(st);

    // Task 3
    st.processorId = 3; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONCE; st.repeatAfterDays = 0;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(100);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    loader.setDBTasks(dbTasks);

    orchestrator.setNextJobsValid(false);
    sapp.RunOnce();
    QVERIFY(orchestrator.getSubmitedJobs().empty());

    orchestrator.setNextJobsValid(true);
    sapp.RunOnce();
    QCOMPARE(orchestrator.getSubmitedJobs().size(), static_cast<size_t>(1));
    QCOMPARE(orchestrator.getSubmitedJobs()[0].processorId, 2);

    sapp.RunOnce();
    QVERIFY(orchestrator.getSubmitedJobs().size() == 1);
}

// check periodicity
void SchedulerTests::testPriority()
{
    TestOrcherstratorProxy orchestrator;
    TestScheduledTaskLoader loader;
    SchedulerApp sapp(0, &loader, &orchestrator);
    ScheduledTask st;
    std::vector<ScheduledTask> dbTasks;

    // Task 1
    st.processorId = 1; st.taskPriority = 3;
    st.repeatType = REPEATTYPE_ONDATE;
    st.repeatOnMonthDay = QDateTime::currentDateTime().date().day();
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(-100);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    // Task 2
    st.processorId = 2; st.taskPriority = 2;
    st.repeatType = REPEATTYPE_CYCLIC; st.repeatAfterDays = 1;
    st.retryPeriod = 0;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(-100);
    dbTasks.push_back(st);

    // Task 3
    st.processorId = 3; st.taskPriority = 1;
    st.repeatType = REPEATTYPE_ONCE; st.repeatAfterDays = 0;
    st.firstScheduledRunTime = QDateTime::currentDateTime().addSecs(-100);
    st.retryPeriod = 1;
    dbTasks.push_back(st);

    loader.setDBTasks(dbTasks);

    sapp.RunOnce();
    QCOMPARE(orchestrator.getSubmitedJobs().size(), static_cast<size_t>(1));
    QCOMPARE(orchestrator.getSubmitedJobs()[0].processorId, 3);

    sapp.RunOnce();
    QCOMPARE(orchestrator.getSubmitedJobs().size(), static_cast<size_t>(2));
    QCOMPARE(orchestrator.getSubmitedJobs()[1].processorId, 2);

    sapp.RunOnce();
    QCOMPARE(orchestrator.getSubmitedJobs().size(), static_cast<size_t>(3));
    QCOMPARE(orchestrator.getSubmitedJobs()[2].processorId, 1);
}

void SchedulerTests::testEstimationTime()
{
    // overload - loadtasks
    // overload - resources
    // overload - running tasks
}
