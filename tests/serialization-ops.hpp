#pragma once

#include <QList>
#include <QTest>

#include "model.hpp"

template <typename T>
struct maker;

template <>
struct maker<bool> {
    static bool make() { return true; }
};

template <>
struct maker<int> {
    static int make() { return 42; }
};

template <>
struct maker<int64_t> {
    static int64_t make() { return 3000000000; }
};

template <>
struct maker<double> {
    static float make() { return 42.12; }
};

template <>
struct maker<QString> {
    static QString make() { return "hello"; }
};

template <>
struct maker<QDateTime> {
    static QDateTime make() { return QDateTime::currentDateTime(); }
};

template <typename T>
struct maker<std::experimental::optional<T>> {
    static std::experimental::optional<T> make() { return maker<T>::make(); }
};

template <typename T>
struct maker<QList<T>> {
    static QList<T> make()
    {
        return QList<T>{ maker<T>::make(), maker<T>::make(), maker<T>::make() };
    }
};

template <>
struct maker<JobStartType> {
    static JobStartType make() { return JobStartType::Scheduled; }
};

template <>
struct maker<EventType> {
    static EventType make() { return EventType::JobResumed; }
};

template <>
struct maker<ExecutionStatus> {
    static ExecutionStatus make() { return ExecutionStatus::Finished; }
};

template <>
struct maker<ProductType> {
    static ProductType make() { return static_cast<ProductType>(1); }
};

template <>
struct maker<ConfigurationParameterInfo> {
    static ConfigurationParameterInfo make()
    {
        return { maker<QString>::make(), maker<int>::make(), maker<QString>::make(),
                 maker<QString>::make(), maker<bool>::make() };
    }
};

template <>
struct maker<ConfigurationParameterValue> {
    static ConfigurationParameterValue make()
    {
        return { maker<QString>::make(), maker<std::experimental::optional<int>>::make(),
                 maker<QString>::make() };
    }
};

template <>
struct maker<JobConfigurationParameterValue> {
    static JobConfigurationParameterValue make()
    {
        return { maker<QString>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<ConfigurationCategory> {
    static ConfigurationCategory make()
    {
        return { maker<int>::make(), maker<QString>::make(), maker<bool>::make() };
    }
};

template <>
struct maker<Site> {
    static Site make() { return { maker<int>::make(), maker<QString>::make(), maker<QString>::make() }; }
};

template <>
struct maker<ConfigurationSet> {
    static ConfigurationSet make()
    {
        return { maker<ConfigurationCategoryList>::make(),
                 maker<ConfigurationParameterInfoList>::make(),
                 maker<ConfigurationParameterValueList>::make(), maker<SiteList>::make(),
                 maker<bool>::make() };
    }
};

template <>
struct maker<ConfigurationUpdateAction> {
    static ConfigurationUpdateAction make()
    {
        return { maker<QString>::make(), maker<std::experimental::optional<int>>::make(),
                 maker<std::experimental::optional<QString>>::make() };
    }
};

template <>
struct maker<JobConfigurationUpdateAction> {
    static JobConfigurationUpdateAction make()
    {
        return { maker<QString>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<KeyedMessage> {
    static KeyedMessage make() { return { maker<QString>::make(), maker<QString>::make() }; }
};

template <>
struct maker<Product> {
    static Product make()
    {
        return { maker<int>::make(), maker<int>::make(),     maker<ProductType>::make(),
                 maker<int>::make(), maker<QString>::make(), maker<QDateTime>::make() };
    }
};

template <>
struct maker<ProductToArchive> {
    static ProductToArchive make()
    {
        return { maker<int>::make(), maker<QString>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<ArchivedProduct> {
    static ArchivedProduct make() { return { maker<int>::make(), maker<QString>::make() }; }
};

template <>
struct maker<NewJob> {
    static NewJob make()
    {
        return { maker<QString>::make(),
                 maker<QString>::make(),
                 maker<int>::make(),
                 maker<int>::make(),
                 maker<JobStartType>::make(),
                 maker<QString>::make(),
                 maker<JobConfigurationUpdateActionList>::make() };
    }
};

template <>
struct maker<NewTask> {
    static NewTask make()
    {
        return { maker<int>::make(), maker<QString>::make(), maker<QString>::make(),
                 maker<TaskIdList>::make() };
    }
};

template <>
struct maker<NewStep> {
    static NewStep make()
    {
        return { maker<int>::make(), maker<QString>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<ExecutionStatistics> {
    static ExecutionStatistics make()
    {
        return { maker<QString>::make(), maker<int32_t>::make(), maker<int64_t>::make(),
                 maker<int64_t>::make(), maker<int64_t>::make(), maker<int32_t>::make(),
                 maker<int32_t>::make(), maker<int64_t>::make(), maker<int64_t>::make(),
                 maker<QString>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<TaskRunnableEvent> {
    static TaskRunnableEvent make()
    {
        return { maker<int>::make(), maker<int>::make(), maker<int>::make() };
    }
};

template <>
struct maker<TaskFinishedEvent> {
    static TaskFinishedEvent make()
    {
        return { maker<int>::make(), maker<int>::make(), maker<int>::make(),
                 maker<int>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<ProductAvailableEvent> {
    static ProductAvailableEvent make() { return { maker<int>::make() }; }
};

template <>
struct maker<JobCancelledEvent> {
    static JobCancelledEvent make() { return { maker<int>::make() }; }
};

template <>
struct maker<JobPausedEvent> {
    static JobPausedEvent make() { return { maker<int>::make() }; }
};

template <>
struct maker<JobResumedEvent> {
    static JobResumedEvent make() { return { maker<int>::make(), maker<int>::make() }; }
};

template <>
struct maker<JobSubmittedEvent> {
    static JobSubmittedEvent make()
    {
        return { maker<int>::make(), maker<int>::make(), maker<int>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<StepFailedEvent> {
    static StepFailedEvent make()
    {
        return { maker<int>::make(), maker<int>::make(), maker<QString>::make() };
    }
};

template <>
struct maker<UnprocessedEvent> {
    static UnprocessedEvent make()
    {
        return { maker<int>::make(), maker<EventType>::make(), maker<QString>::make(),
                 maker<QDateTime>::make(), maker<std::experimental::optional<QDateTime>>::make() };
    }
};

template <>
struct maker<NodeStatistics> {
    static NodeStatistics make()
    {
        return { maker<QString>::make(), maker<double>::make(),  maker<double>::make(),
                 maker<int64_t>::make(), maker<int64_t>::make(), maker<int64_t>::make(),
                 maker<int64_t>::make(), maker<double>::make(),  maker<double>::make(),
                 maker<double>::make(),  maker<int64_t>::make(), maker<int64_t>::make() };
    }
};

template <>
struct maker<StepArgument> {
    static StepArgument make() { return { maker<QString>::make() }; }
};

template <>
struct maker<NewExecutorStep> {
    static NewExecutorStep make()
    {
        return { maker<int>::make(), maker<int>::make(), maker<QString>::make(), maker<QString>::make(),
                 maker<StepArgumentList>::make() };
    }
};

template <>
struct maker<JobStepToRun> {
    static JobStepToRun make()
    {
        return { maker<int>::make(), maker<QString>::make(), maker<QString>::make(),
                 maker<QString>::make() };
    }
};

template <>
struct maker<StepConsoleOutput> {
    static StepConsoleOutput make()
    {
        return { maker<int>::make(), maker<QString>::make(), maker<QString>::make(),
                 maker<QString>::make() };
    }
};

template <>
struct maker<DashboardSearch> {
    static DashboardSearch make()
    {
        return { maker<std::experimental::optional<int>>::make(),
                 maker<std::experimental::optional<int>>::make() };
    }
};

template <typename T>
void compare(const T &v1, const T &v2);

template <typename T>
void compare(const QList<T> &v1, const QList<T> &v2)
{
    QCOMPARE(v1.count(), v2.count());

    auto n = v1.count();
    for (int i = 0; i < n; i++) {
        compare(v1.at(i), v2.at(i));
    }
}

template <typename T>
void compare(const T &v1, const T &v2)
{
    QCOMPARE(v1, v2);
}

void compare(const ConfigurationParameterInfo &v1, const ConfigurationParameterInfo &v2);
void compare(const ConfigurationParameterValue &v1, const ConfigurationParameterValue &v2);
void compare(const JobConfigurationParameterValue &v1, const JobConfigurationParameterValue &v2);
void compare(const ConfigurationCategory &v1, const ConfigurationCategory &v2);
void compare(const Site &v1, const Site &v2);
void compare(const ConfigurationSet &v1, const ConfigurationSet &v2);
void compare(const ConfigurationUpdateAction &v1, const ConfigurationUpdateAction &v2);
void compare(const JobConfigurationUpdateAction &v1, const JobConfigurationUpdateAction &v2);
void compare(const KeyedMessage &v1, const KeyedMessage &v2);
void compare(const Product &v1, const Product &v2);
void compare(const ProductToArchive &v1, const ProductToArchive &v2);
void compare(const ArchivedProduct &v1, const ArchivedProduct &v2);
void compare(const NewJob &v1, const NewJob &v2);
void compare(const NewTask &v1, const NewTask &v2);
void compare(const NewStep &v1, const NewStep &v2);
void compare(const ExecutionStatistics &v1, const ExecutionStatistics &v2);
void compare(const TaskRunnableEvent &v1, const TaskRunnableEvent &v2);
void compare(const TaskFinishedEvent &v1, const TaskFinishedEvent &v2);
void compare(const ProductAvailableEvent &v1, const ProductAvailableEvent &v2);
void compare(const JobCancelledEvent &v1, const JobCancelledEvent &v2);
void compare(const JobPausedEvent &v1, const JobPausedEvent &v2);
void compare(const JobResumedEvent &v1, const JobResumedEvent &v2);
void compare(const JobSubmittedEvent &v1, const JobSubmittedEvent &v2);
void compare(const StepFailedEvent &v1, const StepFailedEvent &v2);
void compare(const UnprocessedEvent &v1, const UnprocessedEvent &v2);
void compare(const NodeStatistics &v1, const NodeStatistics &v2);
void compare(const StepArgument &v1, const StepArgument &v2);
void compare(const NewExecutorStep &v1, const NewExecutorStep &v2);
void compare(const JobStepToRun &v1, const JobStepToRun &v2);
void compare(const StepConsoleOutput &v1, const StepConsoleOutput &v2);
void compare(const DashboardSearch &v1, const DashboardSearch &v2);
