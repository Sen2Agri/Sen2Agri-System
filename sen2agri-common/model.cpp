#include <utility>

#include <QDBusMetaType>
#include <QJsonObject>

#include "model.hpp"

using std::move;

QDBusArgument &operator<<(QDBusArgument &argument, int64_t value);
const QDBusArgument &operator>>(const QDBusArgument &argument, int64_t &value);

QDBusArgument &operator<<(QDBusArgument &argument, uint64_t value);
const QDBusArgument &operator>>(const QDBusArgument &argument, uint64_t &value);

void registerMetaTypes()
{
    ConfigurationSet::registerMetaTypes();

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationParameterValue::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    Site::registerMetaTypes();

    ConfigurationUpdateAction::registerMetaTypes();

    KeyedMessage::registerMetaTypes();

    Product::registerMetaTypes();
    ProductToArchive::registerMetaTypes();
    ArchivedProduct::registerMetaTypes();

    NewJob::registerMetaTypes();
    NewTask::registerMetaTypes();
    NewStep::registerMetaTypes();

    ExecutionStatistics::registerMetaTypes();

    TaskFinishedEvent::registerMetaTypes();
    ProductAvailableEvent::registerMetaTypes();
    JobCancelledEvent::registerMetaTypes();
    JobPausedEvent::registerMetaTypes();
    JobResumedEvent::registerMetaTypes();
    JobSubmittedEvent::registerMetaTypes();
    StepFailedEvent::registerMetaTypes();

    UnprocessedEvent::registerMetaTypes();

    NodeStatistics::registerMetaTypes();

    NewExecutorStep::registerMetaTypes();
    JobStepToRun::registerMetaTypes();

    qDBusRegisterMetaType<QJsonDocument>();

    qDBusRegisterMetaType<JobStartType>();
    qDBusRegisterMetaType<ExecutionStatus>();
    qDBusRegisterMetaType<ExecutionStatusList>();
    qDBusRegisterMetaType<EventType>();

    qDBusRegisterMetaType<StepArgument>();
    qDBusRegisterMetaType<StepArgumentList>();

    qDBusRegisterMetaType<TaskIdList>();
}

ConfigurationParameterInfo::ConfigurationParameterInfo()
{
}

ConfigurationParameterInfo::ConfigurationParameterInfo(
    QString key, int categoryId, QString friendlyName, QString dataType, bool isAdvanced)
    : key(move(key)),
      categoryId(categoryId),
      friendlyName(move(friendlyName)),
      dataType(dataType),
      isAdvanced(isAdvanced)
{
}

void ConfigurationParameterInfo::registerMetaTypes()
{
    qDBusRegisterMetaType<ConfigurationParameterInfo>();
    qDBusRegisterMetaType<ConfigurationParameterInfoList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameterInfo &parameterInfo)
{
    argument.beginStructure();
    argument << parameterInfo.key << parameterInfo.categoryId << parameterInfo.friendlyName
             << parameterInfo.dataType << parameterInfo.isAdvanced;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterInfo &parameterInfo)
{
    argument.beginStructure();
    argument >> parameterInfo.key >> parameterInfo.categoryId >> parameterInfo.friendlyName >>
        parameterInfo.dataType >> parameterInfo.isAdvanced;
    argument.endStructure();

    return argument;
}

ConfigurationParameterValue::ConfigurationParameterValue()
{
}

ConfigurationParameterValue::ConfigurationParameterValue(QString key,
                                                         std::experimental::optional<int> siteId,
                                                         QString value)
    : key(move(key)), siteId(siteId), value(move(value))
{
}

void ConfigurationParameterValue::registerMetaTypes()
{
    qDBusRegisterMetaType<ConfigurationParameterValue>();
    qDBusRegisterMetaType<ConfigurationParameterValueList>();
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const ConfigurationParameterValue &parameterValue)
{
    argument.beginStructure();
    argument << parameterValue.key << parameterValue.siteId.value_or(0) << parameterValue.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterValue &parameterValue)
{
    int siteId;

    argument.beginStructure();
    argument >> parameterValue.key >> siteId >> parameterValue.value;
    argument.endStructure();

    if (siteId) {
        parameterValue.siteId = siteId;
    } else {
        parameterValue.siteId = std::experimental::nullopt;
    }

    return argument;
}

ConfigurationCategory::ConfigurationCategory()
{
}

ConfigurationCategory::ConfigurationCategory(int categoryId, QString name)
    : categoryId(move(categoryId)), name(move(name))
{
}

void ConfigurationCategory::registerMetaTypes()
{
    qDBusRegisterMetaType<ConfigurationCategory>();
    qDBusRegisterMetaType<ConfigurationCategoryList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationCategory &category)
{
    argument.beginStructure();
    argument << category.categoryId << category.name;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationCategory &category)
{
    argument.beginStructure();
    argument >> category.categoryId >> category.name;
    argument.endStructure();

    return argument;
}

Site::Site()
{
}

Site::Site(int siteId, QString name) : siteId(siteId), name(std::move(name))
{
}

void Site::registerMetaTypes()
{
    qDBusRegisterMetaType<Site>();
    qDBusRegisterMetaType<SiteList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const Site &site)
{
    argument.beginStructure();
    argument << site.siteId << site.name;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Site &site)
{
    argument.beginStructure();
    argument >> site.siteId >> site.name;
    argument.endStructure();

    return argument;
}

ConfigurationSet::ConfigurationSet()
{
}

ConfigurationSet::ConfigurationSet(ConfigurationCategoryList categories,
                                   ConfigurationParameterInfoList parameterInfo,
                                   ConfigurationParameterValueList parameterValues,
                                   SiteList sites,
                                   bool isAdmin)
    : categories(std::move(categories)),
      parameterInfo(std::move(parameterInfo)),
      parameterValues(std::move(parameterValues)),
      sites(std::move(sites)),
      isAdmin(isAdmin)
{
}

void ConfigurationSet::registerMetaTypes()
{
    qDBusRegisterMetaType<ConfigurationSet>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationSet &configuration)
{
    argument.beginStructure();
    argument << configuration.categories << configuration.parameterInfo
             << configuration.parameterValues << configuration.sites << configuration.isAdmin;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationSet &configuration)
{
    argument.beginStructure();
    argument >> configuration.categories >> configuration.parameterInfo >>
        configuration.parameterValues >> configuration.sites >> configuration.isAdmin;
    argument.endStructure();

    return argument;
}

ConfigurationUpdateAction::ConfigurationUpdateAction()
{
}

ConfigurationUpdateAction::ConfigurationUpdateAction(QString key,
                                                     std::experimental::optional<int> siteId,
                                                     std::experimental::optional<QString> value)
    : key(std::move(key)), siteId(siteId), value(std::move(value))
{
}

void ConfigurationUpdateAction::registerMetaTypes()
{
    qDBusRegisterMetaType<ConfigurationUpdateAction>();
    qDBusRegisterMetaType<ConfigurationUpdateActionList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationUpdateAction &action)
{
    argument.beginStructure();
    argument << action.key << action.siteId.value_or(0);
    if (action.value) {
        argument << true << action.value.value();
    } else {
        argument << false << QString();
    }
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationUpdateAction &action)
{
    int siteId;
    bool hasValue;
    QString value;

    argument.beginStructure();
    argument >> action.key >> siteId >> hasValue >> value;
    argument.endStructure();

    if (siteId) {
        action.siteId = siteId;
    } else {
        action.siteId = std::experimental::nullopt;
    }

    if (hasValue) {
        action.value = value;
    } else {
        action.value = std::experimental::nullopt;
    }

    return argument;
}

KeyedMessage::KeyedMessage()
{
}

KeyedMessage::KeyedMessage(QString key, QString message)
    : key(std::move(key)), text(std::move(message))
{
}

void KeyedMessage::registerMetaTypes()
{
    qDBusRegisterMetaType<KeyedMessage>();
    qDBusRegisterMetaType<KeyedMessageList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const KeyedMessage &message)
{
    argument.beginStructure();
    argument << message.key << message.text;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KeyedMessage &message)
{
    argument.beginStructure();
    argument >> message.key >> message.text;
    argument.endStructure();

    return argument;
}

Product::Product() : productId(), processorId(), productTypeId(), siteId()
{
}

Product::Product(int productId,
                 int processorId,
                 int productTypeId,
                 int siteId,
                 QString fullPath,
                 QDateTime created)
    : productId(productId),
      processorId(processorId),
      productTypeId(productTypeId),
      siteId(siteId),
      fullPath(std::move(fullPath)),
      created(std::move(created))
{
}

void Product::registerMetaTypes()
{
    qDBusRegisterMetaType<Product>();
    qDBusRegisterMetaType<ProductList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const Product &product)
{
    argument.beginStructure();
    argument << product.productId << product.processorId << product.productTypeId << product.siteId
             << product.fullPath << product.created;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, Product &product)
{
    argument.beginStructure();
    argument >> product.productId >> product.processorId >> product.productTypeId >>
        product.siteId >> product.fullPath >> product.created;
    argument.endStructure();

    return argument;
}

ProductToArchive::ProductToArchive() : productId()
{
}

ProductToArchive::ProductToArchive(int productId, QString currentPath, QString archivePath)
    : productId(productId), currentPath(std::move(currentPath)), archivePath(std::move(archivePath))
{
}

void ProductToArchive::registerMetaTypes()
{
    qDBusRegisterMetaType<ProductToArchive>();
    qDBusRegisterMetaType<ProductToArchiveList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ProductToArchive &product)
{
    argument.beginStructure();
    argument << product.productId << product.currentPath << product.archivePath;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProductToArchive &product)
{
    argument.beginStructure();
    argument >> product.productId >> product.currentPath >> product.archivePath;
    argument.endStructure();

    return argument;
}

ArchivedProduct::ArchivedProduct() : productId(), archivePath()
{
}

ArchivedProduct::ArchivedProduct(int productId, QString archivePath)
    : productId(productId), archivePath(std::move(archivePath))
{
}

void ArchivedProduct::registerMetaTypes()
{
    qDBusRegisterMetaType<ArchivedProduct>();
    qDBusRegisterMetaType<ArchivedProductList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ArchivedProduct &product)
{
    argument.beginStructure();
    argument << product.productId << product.archivePath;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ArchivedProduct &product)
{
    argument.beginStructure();
    argument >> product.productId >> product.archivePath;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, JobStartType startType)
{
    return argument << static_cast<int>(startType);
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobStartType &startType)
{
    int startTypeValue;
    argument >> startTypeValue;
    startType = static_cast<JobStartType>(startTypeValue);
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const QJsonDocument &document)
{
    return argument << document.toBinaryData();
}

const QDBusArgument &operator>>(const QDBusArgument &argument, QJsonDocument &document)
{
    QByteArray data;
    argument >> data;
    document = QJsonDocument::fromBinaryData(data);
    return argument;
}

NewJob::NewJob() : processorId(), siteId(), startType()
{
}

NewJob::NewJob(QString name,
               QString description,
               int processorId,
               int siteId,
               JobStartType startType,
               QJsonDocument parameters)
    : name(std::move(name)),
      description(std::move(description)),
      processorId(processorId),
      siteId(siteId),
      startType(startType),
      parameters(std::move(parameters))
{
}

void NewJob::registerMetaTypes()
{
    qDBusRegisterMetaType<NewJob>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NewJob &job)
{
    argument.beginStructure();
    argument << job.name << job.description << job.processorId << job.siteId << job.startType
             << job.parameters;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewJob &job)
{
    argument.beginStructure();
    argument >> job.name >> job.description >> job.processorId >> job.siteId >> job.startType >>
        job.parameters;
    argument.endStructure();

    return argument;
}

NewTask::NewTask() : jobId()
{
}

NewTask::NewTask(int jobId, QString module, QJsonDocument parameters)
    : jobId(jobId), module(std::move(module)), parameters(std::move(parameters))
{
}

void NewTask::registerMetaTypes()
{
    qDBusRegisterMetaType<NewTask>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NewTask &task)
{
    argument.beginStructure();
    argument << task.jobId << task.module << task.parameters;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewTask &task)
{
    argument.beginStructure();
    argument >> task.jobId >> task.module >> task.parameters;
    argument.endStructure();

    return argument;
}

NewStep::NewStep()
{
}

NewStep::NewStep(QString name, QJsonDocument parameters)
    : name(std::move(name)), parameters(std::move(parameters))
{
}

void NewStep::registerMetaTypes()
{
    qDBusRegisterMetaType<NewStep>();
    qDBusRegisterMetaType<NewStepList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NewStep &step)
{
    argument.beginStructure();
    argument << step.name << step.parameters;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewStep &step)
{
    argument.beginStructure();
    argument >> step.name >> step.parameters;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, ExecutionStatus status)
{
    return argument << static_cast<int>(status);
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ExecutionStatus &status)
{
    int statusValue;
    argument >> statusValue;
    status = static_cast<ExecutionStatus>(statusValue);
    return argument;
}

ExecutionStatistics::ExecutionStatistics()
    : exitCode(),
      userCpuMs(),
      systemCpuMs(),
      durationMs(),
      maxRssKb(),
      maxVmSizeKb(),
      diskReadBytes(),
      diskWriteBytes()
{
}

ExecutionStatistics::ExecutionStatistics(QString node,
                                         int32_t exitCode,
                                         int64_t userCpuMs,
                                         int64_t systemCpuMs,
                                         int64_t durationMs,
                                         int32_t maxRssKb,
                                         int32_t maxVmSizeKb,
                                         int64_t diskReadBytes,
                                         int64_t diskWriteBytes)
    : node(std::move(node)),
      exitCode(exitCode),
      userCpuMs(userCpuMs),
      systemCpuMs(systemCpuMs),
      durationMs(durationMs),
      maxRssKb(maxRssKb),
      maxVmSizeKb(maxVmSizeKb),
      diskReadBytes(diskReadBytes),
      diskWriteBytes(diskWriteBytes)
{
}

void ExecutionStatistics::registerMetaTypes()
{
    qDBusRegisterMetaType<ExecutionStatistics>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ExecutionStatistics &statistics)
{
    argument.beginStructure();
    argument << statistics.node << statistics.exitCode << statistics.userCpuMs
             << statistics.systemCpuMs << statistics.durationMs << statistics.maxRssKb
             << statistics.maxVmSizeKb << statistics.diskReadBytes << statistics.diskWriteBytes;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ExecutionStatistics &statistics)
{
    argument.beginStructure();
    argument >> statistics.node >> statistics.exitCode >> statistics.userCpuMs >>
        statistics.systemCpuMs >> statistics.durationMs >> statistics.maxRssKb >>
        statistics.maxVmSizeKb >> statistics.diskReadBytes >> statistics.diskWriteBytes;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, EventType event)
{
    return argument << static_cast<int>(event);
}

const QDBusArgument &operator>>(const QDBusArgument &argument, EventType &event)
{
    int eventValue;
    argument >> eventValue;
    event = static_cast<EventType>(eventValue);
    return argument;
}

TaskFinishedEvent::TaskFinishedEvent() : taskId()
{
}

TaskFinishedEvent::TaskFinishedEvent(int taskId) : taskId(taskId)
{
}

QJsonDocument TaskFinishedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("task_id")] = taskId;

    return QJsonDocument(node);
}

TaskFinishedEvent TaskFinishedEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("task_id")).toInt() };
}

void TaskFinishedEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<TaskFinishedEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const TaskFinishedEvent &event)
{
    argument.beginStructure();
    argument << event.taskId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, TaskFinishedEvent &event)
{
    argument.beginStructure();
    argument >> event.taskId;
    argument.endStructure();

    return argument;
}

ProductAvailableEvent::ProductAvailableEvent() : productId()
{
}

ProductAvailableEvent::ProductAvailableEvent(int productId) : productId(productId)
{
}

QJsonDocument ProductAvailableEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("product_id")] = productId;

    return QJsonDocument(node);
}

ProductAvailableEvent ProductAvailableEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("product_id")).toInt() };
}

void ProductAvailableEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<ProductAvailableEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const ProductAvailableEvent &event)
{
    argument.beginStructure();
    argument << event.productId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProductAvailableEvent &event)
{
    argument.beginStructure();
    argument >> event.productId;
    argument.endStructure();

    return argument;
}

JobCancelledEvent::JobCancelledEvent() : jobId()
{
}

JobCancelledEvent::JobCancelledEvent(int jobId) : jobId(jobId)
{
}

QJsonDocument JobCancelledEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;

    return QJsonDocument(node);
}

JobCancelledEvent JobCancelledEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("job_id")).toInt() };
}

void JobCancelledEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<JobCancelledEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobCancelledEvent &event)
{
    argument.beginStructure();
    argument << event.jobId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobCancelledEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId;
    argument.endStructure();

    return argument;
}

JobPausedEvent::JobPausedEvent() : jobId()
{
}

JobPausedEvent::JobPausedEvent(int jobId) : jobId(jobId)
{
}

QJsonDocument JobPausedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;

    return QJsonDocument(node);
}

JobPausedEvent JobPausedEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("job_id")).toInt() };
}

void JobPausedEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<JobPausedEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobPausedEvent &event)
{
    argument.beginStructure();
    argument << event.jobId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobPausedEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId;
    argument.endStructure();

    return argument;
}

JobResumedEvent::JobResumedEvent() : jobId()
{
}

JobResumedEvent::JobResumedEvent(int jobId) : jobId(jobId)
{
}

QJsonDocument JobResumedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;

    return QJsonDocument(node);
}

JobResumedEvent JobResumedEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("job_id")).toInt() };
}

void JobResumedEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<JobResumedEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobResumedEvent &event)
{
    argument.beginStructure();
    argument << event.jobId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobResumedEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId;
    argument.endStructure();

    return argument;
}

JobSubmittedEvent::JobSubmittedEvent() : jobId(), processorId()
{
}

JobSubmittedEvent::JobSubmittedEvent(int jobId, int processorId)
    : jobId(jobId), processorId(processorId)
{
}

QJsonDocument JobSubmittedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("processor_id")] = jobId;

    return QJsonDocument(node);
}

JobSubmittedEvent JobSubmittedEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("processor_id")).toInt() };
}

void JobSubmittedEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<JobSubmittedEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobSubmittedEvent &event)
{
    argument.beginStructure();
    argument << event.jobId << event.processorId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobSubmittedEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId >> event.processorId;
    argument.endStructure();

    return argument;
}

StepFailedEvent::StepFailedEvent() : jobId(), taskId()
{
}

StepFailedEvent::StepFailedEvent(int jobId, int taskId, QString stepName)
    : jobId(jobId), taskId(taskId), stepName(std::move(stepName))
{
}

QJsonDocument StepFailedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("task_id")] = taskId;
    node[QStringLiteral("step_name")] = stepName;

    return QJsonDocument(node);
}

StepFailedEvent StepFailedEvent::fromJson(const QJsonDocument &json)
{
    const auto &object = json.object();
    return { object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("task_id")).toInt(),
             object.value(QStringLiteral("step_name")).toString() };
}

void StepFailedEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<StepFailedEvent>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const StepFailedEvent &event)
{
    argument.beginStructure();
    argument << event.jobId << event.taskId << event.stepName;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, StepFailedEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId >> event.taskId >> event.stepName;
    argument.endStructure();

    return argument;
}

UnprocessedEvent::UnprocessedEvent() : eventId(), type()
{
}

UnprocessedEvent::UnprocessedEvent(int eventId,
                                   EventType type,
                                   QJsonDocument data,
                                   QDateTime submittedTime,
                                   std::experimental::optional<QDateTime> processingStartedTime)
    : eventId(eventId),
      type(type),
      data(std::move(data)),
      submittedTime(std::move(submittedTime)),
      processingStartedTime(std::move(processingStartedTime))
{
}

void UnprocessedEvent::registerMetaTypes()
{
    qDBusRegisterMetaType<UnprocessedEvent>();
    qDBusRegisterMetaType<UnprocessedEventList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const UnprocessedEvent &event)
{
    argument.beginStructure();
    argument << event.eventId << event.type << event.data << event.submittedTime;

    if (event.processingStartedTime) {
        argument << true << *event.processingStartedTime;
    } else {
        argument << false << QDateTime();
    }
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, UnprocessedEvent &event)
{
    bool hasProcessingStartedTime;
    QDateTime processingStartedTime;

    argument.beginStructure();
    argument >> event.eventId >> event.type >> event.data >> event.submittedTime >>
        hasProcessingStartedTime >> processingStartedTime;

    if (hasProcessingStartedTime) {
        event.processingStartedTime = std::move(processingStartedTime);
    } else {
        event.processingStartedTime = std::experimental::nullopt;
    }
    argument.endStructure();

    return argument;
}

NodeStatistics::NodeStatistics() : freeRamKb(), freeDiskBytes()
{
}

NodeStatistics::NodeStatistics(QString node, int32_t freeRamKb, int64_t freeDiskBytes)
    : node(std::move(node)), freeRamKb(freeRamKb), freeDiskBytes(freeDiskBytes)
{
}

void NodeStatistics::registerMetaTypes()
{
    qDBusRegisterMetaType<NodeStatistics>();
    qDBusRegisterMetaType<NodeStatisticsList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NodeStatistics &statistics)
{
    argument.beginStructure();
    argument << statistics.node << statistics.freeRamKb << statistics.freeDiskBytes;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NodeStatistics &statistics)
{
    argument.beginStructure();
    argument >> statistics.node >> statistics.freeRamKb >> statistics.freeDiskBytes;
    argument.endStructure();

    return argument;
}

NewExecutorStep::NewExecutorStep() : taskId()
{
}

NewExecutorStep::NewExecutorStep(int taskId,
                                 QString processorPath,
                                 QString stepName,
                                 StepArgumentList arguments)
    : taskId(taskId),
      processorPath(std::move(processorPath)),
      stepName(std::move(stepName)),
      arguments(std::move(arguments))
{
}

void NewExecutorStep::registerMetaTypes()
{
    qDBusRegisterMetaType<StepArgument>();
    qDBusRegisterMetaType<StepArgumentList>();
    qDBusRegisterMetaType<NewExecutorStep>();
    qDBusRegisterMetaType<NewExecutorStepList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NewExecutorStep &step)
{
    argument.beginStructure();
    argument << step.taskId << step.processorPath << step.stepName << step.arguments;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewExecutorStep &step)
{
    argument.beginStructure();
    argument >> step.taskId >> step.processorPath >> step.stepName >> step.arguments;
    argument.endStructure();

    return argument;
}

JobStepToRun::JobStepToRun()
{
}

JobStepToRun::JobStepToRun(int taskId, QString module, QString stepName, QJsonDocument parameters)
    : taskId(taskId),
      module(std::move(module)),
      stepName(std::move(stepName)),
      parameters(std::move(parameters))
{
}

void JobStepToRun::registerMetaTypes()
{
    qDBusRegisterMetaType<JobStepToRun>();
    qDBusRegisterMetaType<JobStepToRunList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobStepToRun &step)
{
    argument.beginStructure();
    argument << step.taskId << step.module << step.stepName << step.parameters;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobStepToRun &step)
{
    argument.beginStructure();
    argument >> step.taskId >> step.module >> step.stepName >> step.parameters;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, int64_t value)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    return argument << qlonglong{ value };
}

const QDBusArgument &operator>>(const QDBusArgument &argument, int64_t &value)
{
    static_assert(sizeof(qlonglong) == sizeof(int64_t), "qlonglong must be 64-bit");

    qlonglong val;

    argument >> val;
    value = val;

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, uint64_t value)
{
    static_assert(sizeof(qulonglong) == sizeof(uint64_t), "qulonglong must be 64-bit");

    return argument << qulonglong{ value };
}

const QDBusArgument &operator>>(const QDBusArgument &argument, uint64_t &value)
{
    static_assert(sizeof(qulonglong) == sizeof(uint64_t), "qulonglong must be 64-bit");

    qulonglong val;

    argument >> val;
    value = val;

    return argument;
}
