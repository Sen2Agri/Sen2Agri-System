#include <utility>

#include <QDBusMetaType>
#include <QJsonObject>
#include <QJsonDocument>

#include "json_conversions.hpp"
#include "model.hpp"

using std::move;

QDBusArgument &operator<<(QDBusArgument &argument, int64_t value);
const QDBusArgument &operator>>(const QDBusArgument &argument, int64_t &value);

QDBusArgument &operator<<(QDBusArgument &argument, uint64_t value);
const QDBusArgument &operator>>(const QDBusArgument &argument, uint64_t &value);

void registerMetaTypes()
{
    ConfigurationSet::registerMetaTypes();

    ConfigurationParameterValue::registerMetaTypes();
    JobConfigurationParameterValue::registerMetaTypes();

    ConfigurationParameterInfo::registerMetaTypes();
    ConfigurationCategory::registerMetaTypes();
    Site::registerMetaTypes();

    ConfigurationUpdateAction::registerMetaTypes();
    JobConfigurationUpdateAction::registerMetaTypes();

    KeyedMessage::registerMetaTypes();

    Product::registerMetaTypes();
    ProductToArchive::registerMetaTypes();
    ArchivedProduct::registerMetaTypes();

    NewJob::registerMetaTypes();
    NewTask::registerMetaTypes();
    NewStep::registerMetaTypes();

    ExecutionStatistics::registerMetaTypes();

    TaskRunnableEvent::registerMetaTypes();
    TaskFinishedEvent::registerMetaTypes();
    ProductAvailableEvent::registerMetaTypes();
    JobCancelledEvent::registerMetaTypes();
    JobPausedEvent::registerMetaTypes();
    JobResumedEvent::registerMetaTypes();
    JobSubmittedEvent::registerMetaTypes();
    StepFailedEvent::registerMetaTypes();

    UnprocessedEvent::registerMetaTypes();

    NodeStatistics::registerMetaTypes();

    StepArgument::registerMetaTypes();
    NewExecutorStep::registerMetaTypes();
    JobStepToRun::registerMetaTypes();
    StepConsoleOutput::registerMetaTypes();

    NewProduct::registerMetaTypes();

    DashboardSearch::registerMetaTypes();

    qDBusRegisterMetaType<ExecutionStatusList>();

    qDBusRegisterMetaType<StepArgument>();
    qDBusRegisterMetaType<StepArgumentList>();

    qDBusRegisterMetaType<TaskIdList>();

    qDBusRegisterMetaType<ProcessingRequest>();
    qDBusRegisterMetaType<JobDefinition>();
}

ConfigurationParameterInfo::ConfigurationParameterInfo() : categoryId(), isAdvanced() {}

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

ConfigurationParameterValue::ConfigurationParameterValue() {}

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

JobConfigurationParameterValue::JobConfigurationParameterValue() {}

JobConfigurationParameterValue::JobConfigurationParameterValue(QString key, QString value)
    : key(move(key)), value(move(value))
{
}

void JobConfigurationParameterValue::registerMetaTypes()
{
    qDBusRegisterMetaType<JobConfigurationParameterValue>();
    qDBusRegisterMetaType<JobConfigurationParameterValueList>();
}

QDBusArgument &operator<<(QDBusArgument &argument,
                          const JobConfigurationParameterValue &parameterValue)
{
    argument.beginStructure();
    argument << parameterValue.key << parameterValue.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument,
                                JobConfigurationParameterValue &parameterValue)
{
    argument.beginStructure();
    argument >> parameterValue.key >> parameterValue.value;
    argument.endStructure();

    return argument;
}

ConfigurationCategory::ConfigurationCategory() : categoryId(), allowPerSiteCustomization() {}

ConfigurationCategory::ConfigurationCategory(int categoryId,
                                             QString name,
                                             bool allowPerSiteCustomization)
    : categoryId(categoryId), name(move(name)), allowPerSiteCustomization(allowPerSiteCustomization)
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
    argument << category.categoryId << category.name << category.allowPerSiteCustomization;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationCategory &category)
{
    argument.beginStructure();
    argument >> category.categoryId >> category.name >> category.allowPerSiteCustomization;
    argument.endStructure();

    return argument;
}

Site::Site() : siteId() {}

Site::Site(int siteId, QString name) : siteId(siteId), name(std::move(name)) {}

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

ConfigurationSet::ConfigurationSet() : isAdmin() {}

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

void ConfigurationSet::registerMetaTypes() { qDBusRegisterMetaType<ConfigurationSet>(); }

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

ConfigurationUpdateAction::ConfigurationUpdateAction() {}

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

JobConfigurationUpdateAction::JobConfigurationUpdateAction() {}

JobConfigurationUpdateAction::JobConfigurationUpdateAction(QString key, QString value)
    : key(std::move(key)), value(std::move(value))
{
}

void JobConfigurationUpdateAction::registerMetaTypes()
{
    qDBusRegisterMetaType<JobConfigurationUpdateAction>();
    qDBusRegisterMetaType<JobConfigurationUpdateActionList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobConfigurationUpdateAction &action)
{
    argument.beginStructure();
    argument << action.key << action.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobConfigurationUpdateAction &action)
{
    argument.beginStructure();
    argument >> action.key >> action.value;
    argument.endStructure();

    return argument;
}

KeyedMessage::KeyedMessage() {}

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

Product::Product() : productId(), processorId(), productTypeId(), siteId() {}

Product::Product(int productId,
                 int processorId,
                 ProductType productTypeId,
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

ProductToArchive::ProductToArchive() : productId() {}

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

ArchivedProduct::ArchivedProduct() : productId(), archivePath() {}

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

QDBusArgument &operator<<(QDBusArgument &argument, const ExecutionStatusList &statusList)
{
    argument.beginArray(qMetaTypeId<int>());
    for (auto s : statusList) {
        argument << s;
    }
    argument.endArray();

    return argument;
}

NewJob::NewJob() : processorId(), siteId(), startType() {}

NewJob::NewJob(QString name,
               QString description,
               int processorId,
               int siteId,
               JobStartType startType,
               QString parametersJson,
               JobConfigurationUpdateActionList configurationOverrides)
    : name(std::move(name)),
      description(std::move(description)),
      processorId(processorId),
      siteId(siteId),
      startType(startType),
      parametersJson(std::move(parametersJson)),
      configuration(std::move(configurationOverrides))
{
}

void NewJob::registerMetaTypes() { qDBusRegisterMetaType<NewJob>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const NewJob &job)
{
    argument.beginStructure();
    argument << job.name << job.description << job.processorId << job.siteId << job.startType
             << job.parametersJson << job.configuration;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewJob &job)
{
    argument.beginStructure();
    argument >> job.name >> job.description >> job.processorId >> job.siteId >> job.startType >>
        job.parametersJson >> job.configuration;
    argument.endStructure();

    return argument;
}

NewTask::NewTask() : jobId() {}

NewTask::NewTask(int jobId, QString module, QString parametersJson, TaskIdList parentTasks)
    : jobId(jobId),
      module(std::move(module)),
      parametersJson(std::move(parametersJson)),
      parentTasks(std::move(parentTasks))
{
}

void NewTask::registerMetaTypes() { qDBusRegisterMetaType<NewTask>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const NewTask &task)
{
    argument.beginStructure();
    argument << task.jobId << task.module << task.parametersJson << task.parentTasks;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewTask &task)
{
    argument.beginStructure();
    argument >> task.jobId >> task.module >> task.parametersJson >> task.parentTasks;
    argument.endStructure();

    return argument;
}

NewStep::NewStep() : taskId() {}

NewStep::NewStep(int taskId, QString name, QString parametersJson)
    : taskId(taskId), name(std::move(name)), parametersJson(std::move(parametersJson))
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
    argument << step.taskId << step.name << step.parametersJson;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewStep &step)
{
    argument.beginStructure();
    argument >> step.taskId >> step.name >> step.parametersJson;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ExecutionStatusList &statusList)
{
    statusList.clear();

    argument.beginArray();
    while (!argument.atEnd()) {
        ExecutionStatus s;
        argument >> s;

        statusList.push_back(s);
    }
    argument.endArray();

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
                                         int64_t diskWriteBytes,
                                         QString stdOutText,
                                         QString stdErrText)
    : node(std::move(node)),
      exitCode(exitCode),
      userCpuMs(userCpuMs),
      systemCpuMs(systemCpuMs),
      durationMs(durationMs),
      maxRssKb(maxRssKb),
      maxVmSizeKb(maxVmSizeKb),
      diskReadBytes(diskReadBytes),
      diskWriteBytes(diskWriteBytes),
      stdOutText(std::move(stdOutText)),
      stdErrText(std::move(stdErrText))
{
}

void ExecutionStatistics::registerMetaTypes() { qDBusRegisterMetaType<ExecutionStatistics>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const ExecutionStatistics &statistics)
{
    argument.beginStructure();
    argument << statistics.node << statistics.exitCode << statistics.userCpuMs
             << statistics.systemCpuMs << statistics.durationMs << statistics.maxRssKb
             << statistics.maxVmSizeKb << statistics.diskReadBytes << statistics.diskWriteBytes
             << statistics.stdOutText << statistics.stdErrText;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ExecutionStatistics &statistics)
{
    argument.beginStructure();
    argument >> statistics.node >> statistics.exitCode >> statistics.userCpuMs >>
        statistics.systemCpuMs >> statistics.durationMs >> statistics.maxRssKb >>
        statistics.maxVmSizeKb >> statistics.diskReadBytes >> statistics.diskWriteBytes >>
        statistics.stdOutText >> statistics.stdErrText;
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

TaskRunnableEvent::TaskRunnableEvent() : jobId(), processorId(), taskId() {}

TaskRunnableEvent::TaskRunnableEvent(int jobId, int processorId, int taskId)
    : jobId(jobId), processorId(processorId), taskId(taskId)
{
}

QString TaskRunnableEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("processor_id")] = processorId;
    node[QStringLiteral("task_id")] = taskId;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

TaskRunnableEvent TaskRunnableEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("processor_id")).toInt(),
             object.value(QStringLiteral("task_id")).toInt() };
}

void TaskRunnableEvent::registerMetaTypes() { qDBusRegisterMetaType<TaskRunnableEvent>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const TaskRunnableEvent &event)
{
    argument.beginStructure();
    argument << event.jobId << event.taskId << event.processorId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, TaskRunnableEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId >> event.taskId >> event.processorId;
    argument.endStructure();

    return argument;
}

TaskFinishedEvent::TaskFinishedEvent() : processorId(), siteId(), jobId(), taskId() {}

TaskFinishedEvent::TaskFinishedEvent(int processorId, int siteId, int jobId, int taskId, QString module)
    : processorId(processorId), siteId(siteId), jobId(jobId), taskId(taskId), module(std::move(module))
{
}

QString TaskFinishedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("processor_id")] = processorId;
    node[QStringLiteral("site_id")] = siteId;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("task_id")] = taskId;
    node[QStringLiteral("module_short_name")] = module;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

TaskFinishedEvent TaskFinishedEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("processor_id")).toInt(),
             object.value(QStringLiteral("site_id")).toInt(),
             object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("task_id")).toInt(),
             object.value(QStringLiteral("module_short_name")).toString() };
}

void TaskFinishedEvent::registerMetaTypes() { qDBusRegisterMetaType<TaskFinishedEvent>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const TaskFinishedEvent &event)
{
    argument.beginStructure();
    argument << event.processorId << event.siteId << event.jobId << event.taskId << event.module;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, TaskFinishedEvent &event)
{
    argument.beginStructure();
    argument >> event.processorId >> event.siteId >> event.jobId >> event.taskId >> event.module;
    argument.endStructure();

    return argument;
}

ProductAvailableEvent::ProductAvailableEvent() : productId() {}

ProductAvailableEvent::ProductAvailableEvent(int productId) : productId(productId) {}

QString ProductAvailableEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("product_id")] = productId;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

ProductAvailableEvent ProductAvailableEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("product_id")).toInt() };
}

void ProductAvailableEvent::registerMetaTypes() { qDBusRegisterMetaType<ProductAvailableEvent>(); }

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

JobCancelledEvent::JobCancelledEvent() : jobId() {}

JobCancelledEvent::JobCancelledEvent(int jobId) : jobId(jobId) {}

QString JobCancelledEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

JobCancelledEvent JobCancelledEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("job_id")).toInt() };
}

void JobCancelledEvent::registerMetaTypes() { qDBusRegisterMetaType<JobCancelledEvent>(); }

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

JobPausedEvent::JobPausedEvent() : jobId() {}

JobPausedEvent::JobPausedEvent(int jobId) : jobId(jobId) {}

QString JobPausedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

JobPausedEvent JobPausedEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("job_id")).toInt() };
}

void JobPausedEvent::registerMetaTypes() { qDBusRegisterMetaType<JobPausedEvent>(); }

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

JobResumedEvent::JobResumedEvent() : jobId() {}

JobResumedEvent::JobResumedEvent(int jobId, int processorId) : jobId(jobId), processorId(processorId) {}

QString JobResumedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("processor_id")] = processorId;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

JobResumedEvent JobResumedEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("processor_id")).toInt() };
}

void JobResumedEvent::registerMetaTypes() { qDBusRegisterMetaType<JobResumedEvent>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const JobResumedEvent &event)
{
    argument.beginStructure();
    argument << event.jobId << event.processorId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobResumedEvent &event)
{
    argument.beginStructure();
    argument >> event.processorId >> event.jobId;
    argument.endStructure();

    return argument;
}

JobSubmittedEvent::JobSubmittedEvent() : jobId(), processorId(), siteId() {}

JobSubmittedEvent::JobSubmittedEvent(int jobId, int processorId, int siteId, QString parametersJson)
    : jobId(jobId), processorId(processorId), siteId(siteId), parametersJson(std::move(parametersJson))
{
}

QString JobSubmittedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("processor_id")] = processorId;
    node[QStringLiteral("site_id")] = siteId;
    node[QStringLiteral("parameters")] = parametersJson;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

JobSubmittedEvent JobSubmittedEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("processor_id")).toInt(),
             object.value(QStringLiteral("site_id")).toInt(),
             jsonToString(object.value(QStringLiteral("parameters")).toObject()) };
}

void JobSubmittedEvent::registerMetaTypes() { qDBusRegisterMetaType<JobSubmittedEvent>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const JobSubmittedEvent &event)
{
    argument.beginStructure();
    argument << event.jobId << event.processorId << event.siteId << event.parametersJson;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobSubmittedEvent &event)
{
    argument.beginStructure();
    argument >> event.jobId >> event.processorId >> event.siteId >> event.parametersJson;
    argument.endStructure();

    return argument;
}

StepFailedEvent::StepFailedEvent() : jobId(), taskId() {}

StepFailedEvent::StepFailedEvent(int jobId, int taskId, QString stepName)
    : jobId(jobId), taskId(taskId), stepName(std::move(stepName))
{
}

QString StepFailedEvent::toJson() const
{
    QJsonObject node;
    node[QStringLiteral("job_id")] = jobId;
    node[QStringLiteral("task_id")] = taskId;
    node[QStringLiteral("step_name")] = stepName;

    return QString::fromUtf8(QJsonDocument(node).toJson());
}

StepFailedEvent StepFailedEvent::fromJson(const QString &json)
{
    const auto &doc = QJsonDocument::fromJson(json.toUtf8());
    const auto &object = doc.object();

    return { object.value(QStringLiteral("job_id")).toInt(),
             object.value(QStringLiteral("task_id")).toInt(),
             object.value(QStringLiteral("step_name")).toString() };
}

void StepFailedEvent::registerMetaTypes() { qDBusRegisterMetaType<StepFailedEvent>(); }

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

UnprocessedEvent::UnprocessedEvent() : eventId(), type() {}

UnprocessedEvent::UnprocessedEvent(int eventId,
                                   EventType type,
                                   QString dataJson,
                                   QDateTime submittedTime,
                                   std::experimental::optional<QDateTime> processingStartedTime)
    : eventId(eventId),
      type(type),
      dataJson(std::move(dataJson)),
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
    argument << event.eventId << event.type << event.dataJson << event.submittedTime;

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
    argument >> event.eventId >> event.type >> event.dataJson >> event.submittedTime >>
        hasProcessingStartedTime >> processingStartedTime;

    if (hasProcessingStartedTime) {
        event.processingStartedTime = std::move(processingStartedTime);
    } else {
        event.processingStartedTime = std::experimental::nullopt;
    }
    argument.endStructure();

    return argument;
}

NodeStatistics::NodeStatistics()
    : cpuUser(),
      cpuSystem(),
      memTotalKb(),
      memUsedKb(),
      swapTotalKb(),
      swapUsedKb(),
      loadAvg1(),
      loadAvg5(),
      loadAvg15(),
      diskTotalBytes(),
      diskUsedBytes()
{
}

NodeStatistics::NodeStatistics(QString node,
                               double cpuUser,
                               double cpuSystem,
                               int64_t memTotalKb,
                               int64_t memUsedKb,
                               int64_t swapTotalKb,
                               int64_t swapUsedKb,
                               double loadAvg1,
                               double loadAvg5,
                               double loadAvg15,
                               int64_t diskTotalBytes,
                               int64_t diskUsedBytes)
    : node(std::move(node)),
      cpuUser(cpuUser),
      cpuSystem(cpuSystem),
      memTotalKb(memTotalKb),
      memUsedKb(memUsedKb),
      swapTotalKb(swapTotalKb),
      swapUsedKb(swapUsedKb),
      loadAvg1(loadAvg1),
      loadAvg5(loadAvg5),
      loadAvg15(loadAvg15),
      diskTotalBytes(diskTotalBytes),
      diskUsedBytes(diskUsedBytes)
{
}

void NodeStatistics::registerMetaTypes() { qDBusRegisterMetaType<NodeStatistics>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const NodeStatistics &statistics)
{
    argument.beginStructure();
    argument << statistics.node << statistics.cpuUser << statistics.cpuSystem
             << statistics.memTotalKb << statistics.memUsedKb << statistics.swapTotalKb
             << statistics.swapUsedKb << statistics.loadAvg1 << statistics.loadAvg5
             << statistics.loadAvg15 << statistics.diskTotalBytes << statistics.diskUsedBytes;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NodeStatistics &statistics)
{
    argument.beginStructure();
    argument >> statistics.node >> statistics.cpuUser >> statistics.cpuSystem >>
        statistics.memTotalKb >> statistics.memUsedKb >> statistics.swapTotalKb >>
        statistics.swapUsedKb >> statistics.loadAvg1 >> statistics.loadAvg5 >>
        statistics.loadAvg15 >> statistics.diskTotalBytes >> statistics.diskUsedBytes;
    argument.endStructure();

    return argument;
}

StepArgument::StepArgument() {}

StepArgument::StepArgument(QString value) : value(std::move(value)) {}

void StepArgument::registerMetaTypes()
{
    qDBusRegisterMetaType<StepArgument>();
    qDBusRegisterMetaType<StepArgumentList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const StepArgument &stepArgument)
{
    argument.beginStructure();
    argument << stepArgument.value;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, StepArgument &stepArgument)
{
    argument.beginStructure();
    argument >> stepArgument.value;
    argument.endStructure();

    return argument;
}

NewExecutorStep::NewExecutorStep() : processorId(), taskId() {}

NewExecutorStep::NewExecutorStep(int processorId,
                                 int taskId,
                                 QString processorPath,
                                 QString stepName,
                                 StepArgumentList arguments)
    : processorId(processorId),
      taskId(taskId),
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
    argument << step.processorId << step.taskId << step.processorPath << step.stepName << step.arguments;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewExecutorStep &step)
{
    argument.beginStructure();
    argument >> step.processorId >> step.taskId >> step.processorPath >> step.stepName >> step.arguments;
    argument.endStructure();

    return argument;
}

JobStepToRun::JobStepToRun() : taskId() {}

JobStepToRun::JobStepToRun(int taskId, QString module, QString stepName, QString parametersJson)
    : taskId(taskId),
      module(std::move(module)),
      stepName(std::move(stepName)),
      parametersJson(std::move(parametersJson))
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
    argument << step.taskId << step.module << step.stepName << step.parametersJson;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobStepToRun &step)
{
    argument.beginStructure();
    argument >> step.taskId >> step.module >> step.stepName >> step.parametersJson;
    argument.endStructure();

    return argument;
}

StepConsoleOutput::StepConsoleOutput() : taskId() {}

StepConsoleOutput::StepConsoleOutput(int taskId,
                                     QString stepName,
                                     QString stdOutText,
                                     QString stdErrText)
    : taskId(taskId),
      stepName(std::move(stepName)),
      stdOutText(std::move(stdOutText)),
      stdErrText(std::move(stdErrText))
{
}

void StepConsoleOutput::registerMetaTypes()
{
    qDBusRegisterMetaType<StepConsoleOutput>();
    qDBusRegisterMetaType<StepConsoleOutputList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const StepConsoleOutput &stepOutput)
{
    argument.beginStructure();
    argument << stepOutput.taskId << stepOutput.stepName << stepOutput.stdOutText
             << stepOutput.stdErrText;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, StepConsoleOutput &stepOutput)
{
    argument.beginStructure();
    argument >> stepOutput.taskId >> stepOutput.stepName >> stepOutput.stdOutText >>
        stepOutput.stdErrText;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const ProductType &productType)
{
    return argument << static_cast<int>(productType);
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProductType &productType)
{
    int productTypeValue;
    argument >> productTypeValue;
    productType = static_cast<ProductType>(productTypeValue);
    return argument;
}

NewProduct::NewProduct() : productType(), processorId(), taskId() {}

NewProduct::NewProduct(ProductType productType,
                       int processorId,
                       int taskId,
                       QString fullPath,
                       QDateTime createdTimestamp)
    : productType(productType),
      processorId(processorId),
      taskId(taskId),
      fullPath(std::move(fullPath)),
      createdTimestamp(std::move(createdTimestamp))
{
}

void NewProduct::registerMetaTypes() { qDBusRegisterMetaType<NewProduct>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const NewProduct &product)
{
    argument.beginStructure();
    argument << product.productType << product.processorId << product.taskId << product.fullPath
             << product.createdTimestamp;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewProduct &product)
{
    argument.beginStructure();
    argument >> product.productType >> product.processorId >> product.taskId >> product.fullPath >>
        product.createdTimestamp;
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

DashboardSearch::DashboardSearch() {}

DashboardSearch::DashboardSearch(std::experimental::optional<int> siteId,
                                 std::experimental::optional<int> processorId)
    : siteId(siteId), processorId(processorId)
{
}

void DashboardSearch::registerMetaTypes() { qDBusRegisterMetaType<DashboardSearch>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const DashboardSearch &search)
{
    argument.beginStructure();
    argument << search.siteId.value_or(0) << search.processorId.value_or(0);
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DashboardSearch &search)
{
    int siteId;
    int processorId;

    argument.beginStructure();
    argument >> siteId >> processorId;
    argument.endStructure();

    if (siteId) {
        search.siteId = siteId;
    } else {
        search.siteId = std::experimental::nullopt;
    }

    if (processorId) {
        search.processorId = processorId;
    } else {
        search.processorId = std::experimental::nullopt;
    }

    return argument;
}

ProcessorDescription::ProcessorDescription() : processorId() {}

ProcessorDescription::ProcessorDescription(
               int processorId,
               QString shortName,
               QString fullName)
    : processorId(processorId),
      shortName(std::move(shortName)),
      fullName(std::move(fullName))
{
}

void ProcessorDescription::registerMetaTypes() { qDBusRegisterMetaType<ProcessorDescription>(); }

QDBusArgument &operator<<(QDBusArgument &argument, const ProcessorDescription &processor)
{
    argument.beginStructure();
    argument << processor.processorId << processor.shortName << processor.fullName;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProcessorDescription &processor)
{
    argument.beginStructure();
    argument >> processor.processorId >> processor.shortName >> processor.fullName;
    argument.endStructure();

    return argument;
}


ScheduledTask::ScheduledTask(int ti, QString tn ,int pi, int si, QString pp ,int rt, int rad, int rmd,
                             QDateTime  fst, int rp, int tp, ScheduledTaskStatus& ts):
 taskId(ti),
 taskName(tn),
 processorId(pi),
 siteId(si),
 processorParameters(pp),
 repeatType(rt),
 repeatAfterDays(rad),
 repeatOnMonthDay(rmd),
 firstScheduledRunTime(fst),
 retryPeriod(rp),
 taskPriority(tp),
 taskStatus(ts)
{
}

QDBusArgument &operator<<(QDBusArgument &argument, const ProcessingRequest &request)
{
    argument.beginStructure();
    argument << request.processorId << request.siteId << request.parametersJson ;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, ProcessingRequest &request)
{
    argument.beginStructure();
    argument >> request.processorId >> request.siteId >> request.parametersJson ;
    argument.endStructure();

    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const JobDefinition &job)
{
    argument.beginStructure();
    argument << job.isValid << job.jobDefinitionJson << job.processorId;
    argument.endStructure();

    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, JobDefinition &job)
{
    argument.beginStructure();
    argument >> job.isValid >> job.jobDefinitionJson >> job.processorId;
    argument.endStructure();

    return argument;
}
