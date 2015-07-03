#pragma once

#include <QObject>

#include "model.hpp"

class Reflector : public QObject
{
    Q_OBJECT

public:
    explicit Reflector(QObject *parent = 0);

public slots:
    ConfigurationParameterInfo
    ReflectConfigurationParameterInfo(const ConfigurationParameterInfo &value);
    ConfigurationParameterInfoList
    ReflectConfigurationParameterInfoList(const ConfigurationParameterInfoList &value);
    ConfigurationParameterValue
    ReflectConfigurationParameterValue(const ConfigurationParameterValue &value);
    ConfigurationParameterValueList
    ReflectConfigurationParameterValueList(const ConfigurationParameterValueList &value);
    ConfigurationCategory ReflectConfigurationCategory(const ConfigurationCategory &value);
    ConfigurationCategoryList
    ReflectConfigurationCategoryList(const ConfigurationCategoryList &value);
    Site ReflectSite(const Site &value);
    SiteList ReflectSiteList(const SiteList &value);
    ConfigurationSet ReflectConfigurationSet(const ConfigurationSet &value);
    ConfigurationUpdateAction
    ReflectConfigurationUpdateAction(const ConfigurationUpdateAction &value);
    ConfigurationUpdateActionList
    ReflectConfigurationUpdateActionList(const ConfigurationUpdateActionList &value);
    KeyedMessage ReflectKeyedMessage(const KeyedMessage &value);
    KeyedMessageList ReflectKeyedMessageList(const KeyedMessageList &value);
    Product ReflectProduct(const Product &value);
    ProductList ReflectProductList(const ProductList &value);
    ProductToArchive ReflectProductToArchive(const ProductToArchive &value);
    ProductToArchiveList ReflectProductToArchiveList(const ProductToArchiveList &value);
    ArchivedProduct ReflectArchivedProduct(const ArchivedProduct &value);
    ArchivedProductList ReflectArchivedProductList(const ArchivedProductList &value);
    NewJob ReflectNewJob(const NewJob &value);
    NewTask ReflectNewTask(const NewTask &value);
    NewStep ReflectNewStep(const NewStep &value);
    NewStepList ReflectNewStepList(const NewStepList &value);
    ExecutionStatusList ReflectExecutionStatusList(const ExecutionStatusList &value);
    ExecutionStatistics ReflectExecutionStatistics(const ExecutionStatistics &value);
    TaskFinishedEvent ReflectTaskFinishedEvent(const TaskFinishedEvent &value);
    ProductAvailableEvent ReflectProductAvailableEvent(const ProductAvailableEvent &value);
    JobCancelledEvent ReflectJobCancelledEvent(const JobCancelledEvent &value);
    JobPausedEvent ReflectJobPausedEvent(const JobPausedEvent &value);
    JobResumedEvent ReflectJobResumedEvent(const JobResumedEvent &value);
    JobSubmittedEvent ReflectJobSubmittedEvent(const JobSubmittedEvent &value);
    StepFailedEvent ReflectStepFailedEvent(const StepFailedEvent &value);
    UnprocessedEvent ReflectUnprocessedEvent(const UnprocessedEvent &value);
    UnprocessedEventList ReflectUnprocessedEventList(const UnprocessedEventList &value);
    NodeStatistics ReflectNodeStatistics(const NodeStatistics &value);
    NodeStatisticsList ReflectNodeStatisticsList(const NodeStatisticsList &value);
    NewExecutorStep ReflectNewExecutorStep(const NewExecutorStep &value);
    NewExecutorStepList ReflectNewExecutorStepList(const NewExecutorStepList &value);
    JobStepToRun ReflectJobStepToRun(const JobStepToRun &value);
    JobStepToRunList ReflectJobStepToRunList(const JobStepToRunList &value);
};
