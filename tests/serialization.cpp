#include <QTest>

#include "make_unique.hpp"
#include "dbus_future_utils.hpp"
#include "serialization.hpp"
#include "serialization-ops.hpp"
#include "reflector.hpp"
#include "reflector_adaptor.h"

void Serialization::initTestCase()
{
    registerMetaTypes();

    auto reflector = new Reflector(this);

    new ReflectorAdaptor(reflector);

    auto connection = QDBusConnection::sessionBus();
    if (!connection.registerObject(reflectorName, reflector)) {
        QFAIL("Error registering the object with D-Bus: %1.");
    }

    if (!connection.registerService(OrgEsaSen2agriReflectorInterface::staticInterfaceName())) {
        QFAIL("Error registering the service with D-Bus: %1.");
    }

    client = std::make_unique<OrgEsaSen2agriReflectorInterface>(
        OrgEsaSen2agriReflectorInterface::staticInterfaceName(), reflectorName, connection);
}

void Serialization::cleanupTestCase()
{
    client.reset();

    auto connection = QDBusConnection::sessionBus();
    connection.unregisterService(OrgEsaSen2agriReflectorInterface::staticInterfaceName());
    connection.unregisterObject(reflectorName);
}

QString Serialization::reflectorName = QStringLiteral("/org/esa/sen2agri/reflector");

void Serialization::configurationParameterInfo()
{
    const auto &value = maker<ConfigurationParameterInfo>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterInfo(value)), value);

    ConfigurationParameterInfo emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterInfo(emptyValue)),
            emptyValue);
}

void Serialization::configurationParameterInfoList()
{
    const auto &value = maker<ConfigurationParameterInfoList>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterInfoList(value)), value);

    ConfigurationParameterInfoList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterInfoList(emptyValue)),
            emptyValue);
}

void Serialization::configurationParameterValue()
{
    const auto &value = maker<ConfigurationParameterValue>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterValue(value)), value);

    ConfigurationParameterValue emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterValue(emptyValue)),
            emptyValue);
}

void Serialization::configurationParameterValueList()
{
    const auto &value = maker<ConfigurationParameterValueList>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterValueList(value)), value);

    ConfigurationParameterValueList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationParameterValueList(emptyValue)),
            emptyValue);
}

void Serialization::configurationCategory()
{
    const auto &value = maker<ConfigurationCategory>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationCategory(value)), value);

    ConfigurationCategory emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationCategory(emptyValue)), emptyValue);
}

void Serialization::configurationCategoryList()
{
    const auto &value = maker<ConfigurationCategoryList>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationCategoryList(value)), value);

    ConfigurationCategoryList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationCategoryList(emptyValue)),
            emptyValue);
}

void Serialization::site()
{
    const auto &value = maker<Site>::make();
    compare(WaitForResponseAndThrow(client->ReflectSite(value)), value);

    Site emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectSite(emptyValue)), emptyValue);
}

void Serialization::siteList()
{
    const auto &value = maker<SiteList>::make();
    compare(WaitForResponseAndThrow(client->ReflectSiteList(value)), value);

    SiteList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectSiteList(emptyValue)), emptyValue);
}

void Serialization::configurationSet()
{
    const auto &value = maker<ConfigurationSet>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationSet(value)), value);

    ConfigurationSet emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationSet(emptyValue)), emptyValue);
}

void Serialization::configurationUpdateAction()
{
    const auto &value = maker<ConfigurationUpdateAction>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationUpdateAction(value)), value);

    ConfigurationUpdateAction emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationUpdateAction(emptyValue)),
            emptyValue);
}

void Serialization::configurationUpdateActionList()
{
    const auto &value = maker<ConfigurationUpdateActionList>::make();
    compare(WaitForResponseAndThrow(client->ReflectConfigurationUpdateActionList(value)), value);

    ConfigurationUpdateActionList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectConfigurationUpdateActionList(emptyValue)),
            emptyValue);
}

void Serialization::keyedMessage()
{
    const auto &value = maker<KeyedMessage>::make();
    compare(WaitForResponseAndThrow(client->ReflectKeyedMessage(value)), value);

    KeyedMessage emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectKeyedMessage(emptyValue)), emptyValue);
}

void Serialization::keyedMessageList()
{
    const auto &value = maker<KeyedMessageList>::make();
    compare(WaitForResponseAndThrow(client->ReflectKeyedMessageList(value)), value);

    KeyedMessageList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectKeyedMessageList(emptyValue)), emptyValue);
}

void Serialization::product()
{
    const auto &value = maker<Product>::make();
    compare(WaitForResponseAndThrow(client->ReflectProduct(value)), value);

    Product emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectProduct(emptyValue)), emptyValue);
}

void Serialization::productList()
{
    const auto &value = maker<ProductList>::make();
    compare(WaitForResponseAndThrow(client->ReflectProductList(value)), value);

    ProductList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectProductList(emptyValue)), emptyValue);
}

void Serialization::productToArchive()
{
    const auto &value = maker<ProductToArchive>::make();
    compare(WaitForResponseAndThrow(client->ReflectProductToArchive(value)), value);

    ProductToArchive emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectProductToArchive(emptyValue)), emptyValue);
}

void Serialization::productToArchiveList()
{
    const auto &value = maker<ProductToArchiveList>::make();
    compare(WaitForResponseAndThrow(client->ReflectProductToArchiveList(value)), value);

    ProductToArchiveList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectProductToArchiveList(emptyValue)), emptyValue);
}

void Serialization::archivedProduct()
{
    const auto &value = maker<ArchivedProduct>::make();
    compare(WaitForResponseAndThrow(client->ReflectArchivedProduct(value)), value);

    ArchivedProduct emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectArchivedProduct(emptyValue)), emptyValue);
}

void Serialization::archivedProductList()
{
    const auto &value = maker<ArchivedProductList>::make();
    compare(WaitForResponseAndThrow(client->ReflectArchivedProductList(value)), value);

    ArchivedProductList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectArchivedProductList(emptyValue)), emptyValue);
}

void Serialization::newJob()
{
    const auto &value = maker<NewJob>::make();
    compare(WaitForResponseAndThrow(client->ReflectNewJob(value)), value);

    NewJob emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNewJob(emptyValue)), emptyValue);
}

void Serialization::newTask()
{
    const auto &value = maker<NewTask>::make();
    compare(WaitForResponseAndThrow(client->ReflectNewTask(value)), value);

    NewTask emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNewTask(emptyValue)), emptyValue);
}

void Serialization::newStep()
{
    const auto &value = maker<NewStep>::make();
    compare(WaitForResponseAndThrow(client->ReflectNewStep(value)), value);

    NewStep emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNewStep(emptyValue)), emptyValue);
}

void Serialization::newStepList()
{
    const auto &value = maker<NewStepList>::make();
    compare(WaitForResponseAndThrow(client->ReflectNewStepList(value)), value);

    NewStepList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNewStepList(emptyValue)), emptyValue);
}

void Serialization::executionStatusList()
{
    const auto &value = maker<ExecutionStatusList>::make();
    compare(WaitForResponseAndThrow(client->ReflectExecutionStatusList(value)), value);

    ExecutionStatusList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectExecutionStatusList(emptyValue)), emptyValue);
}

void Serialization::executionStatistics()
{
    const auto &value = maker<ExecutionStatistics>::make();
    compare(WaitForResponseAndThrow(client->ReflectExecutionStatistics(value)), value);

    ExecutionStatistics emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectExecutionStatistics(emptyValue)), emptyValue);
}

void Serialization::taskFinishedEvent()
{
    const auto &value = maker<TaskFinishedEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectTaskFinishedEvent(value)), value);

    TaskFinishedEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectTaskFinishedEvent(emptyValue)), emptyValue);
}

void Serialization::productAvailableEvent()
{
    const auto &value = maker<ProductAvailableEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectProductAvailableEvent(value)), value);

    ProductAvailableEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectProductAvailableEvent(emptyValue)), emptyValue);
}

void Serialization::jobCancelledEvent()
{
    const auto &value = maker<JobCancelledEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectJobCancelledEvent(value)), value);

    JobCancelledEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectJobCancelledEvent(emptyValue)), emptyValue);
}

void Serialization::jobPausedEvent()
{
    const auto &value = maker<JobPausedEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectJobPausedEvent(value)), value);

    JobPausedEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectJobPausedEvent(emptyValue)), emptyValue);
}

void Serialization::jobResumedEvent()
{
    const auto &value = maker<JobResumedEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectJobResumedEvent(value)), value);

    JobResumedEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectJobResumedEvent(emptyValue)), emptyValue);
}

void Serialization::jobSubmittedEvent()
{
    const auto &value = maker<JobSubmittedEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectJobSubmittedEvent(value)), value);

    JobSubmittedEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectJobSubmittedEvent(emptyValue)), emptyValue);
}

void Serialization::stepFailedEvent()
{
    const auto &value = maker<StepFailedEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectStepFailedEvent(value)), value);

    StepFailedEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectStepFailedEvent(emptyValue)), emptyValue);
}

void Serialization::unprocessedEvent()
{
    const auto &value = maker<UnprocessedEvent>::make();
    compare(WaitForResponseAndThrow(client->ReflectUnprocessedEvent(value)), value);

    UnprocessedEvent emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectUnprocessedEvent(emptyValue)), emptyValue);
}

void Serialization::unprocessedEventList()
{
    const auto &value = maker<UnprocessedEventList>::make();
    compare(WaitForResponseAndThrow(client->ReflectUnprocessedEventList(value)), value);

    UnprocessedEventList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectUnprocessedEventList(emptyValue)), emptyValue);
}

void Serialization::nodeStatistics()
{
    const auto &value = maker<NodeStatistics>::make();
    compare(WaitForResponseAndThrow(client->ReflectNodeStatistics(value)), value);

    NodeStatistics emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNodeStatistics(emptyValue)), emptyValue);
}

void Serialization::nodeStatisticsList()
{
    const auto &value = maker<NodeStatisticsList>::make();
    compare(WaitForResponseAndThrow(client->ReflectNodeStatisticsList(value)), value);

    NodeStatisticsList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNodeStatisticsList(emptyValue)), emptyValue);
}

void Serialization::newExecutorStep()
{
    const auto &value = maker<NewExecutorStep>::make();
    compare(WaitForResponseAndThrow(client->ReflectNewExecutorStep(value)), value);

    NewExecutorStep emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNewExecutorStep(emptyValue)), emptyValue);
}

void Serialization::newExecutorStepList()
{
    const auto &value = maker<NewExecutorStepList>::make();
    compare(WaitForResponseAndThrow(client->ReflectNewExecutorStepList(value)), value);

    NewExecutorStepList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectNewExecutorStepList(emptyValue)), emptyValue);
}

void Serialization::jobStepToRun()
{
    const auto &value = maker<JobStepToRun>::make();
    compare(WaitForResponseAndThrow(client->ReflectJobStepToRun(value)), value);

    JobStepToRun emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectJobStepToRun(emptyValue)), emptyValue);
}

void Serialization::jobStepToRunList()
{
    const auto &value = maker<JobStepToRunList>::make();
    compare(WaitForResponseAndThrow(client->ReflectJobStepToRunList(value)), value);

    JobStepToRunList emptyValue;
    compare(WaitForResponseAndThrow(client->ReflectJobStepToRunList(emptyValue)), emptyValue);
}
