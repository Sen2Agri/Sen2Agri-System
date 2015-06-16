#include <utility>

#include <QDBusMetaType>
#include <QJsonDocument>
#include <QJsonObject>

#include "model.hpp"

using std::move;

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

    qDBusRegisterMetaType<JobStartType>();
    qDBusRegisterMetaType<QJsonDocument>();
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
    return argument << job.name << job.description << job.processorId << job.siteId << job.startType
                    << job.parameters;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewJob &job)
{
    return argument >> job.name >> job.description >> job.processorId >> job.siteId >>
           job.startType >> job.parameters;
}

NewTask::NewTask() : jobId(), moduleId()
{
}

NewTask::NewTask(int jobId, int moduleId, QJsonDocument parameters)
    : jobId(jobId), moduleId(moduleId), parameters(std::move(parameters))
{
}

void NewTask::registerMetaTypes()
{
    qDBusRegisterMetaType<NewTask>();
    qDBusRegisterMetaType<NewTaskList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NewTask &task)
{
    return argument << task.jobId << task.moduleId << task.parameters;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewTask &task)
{
    return argument >> task.jobId >> task.moduleId >> task.parameters;
}

NewStep::NewStep() : taskId()
{
}

NewStep::NewStep(int taskId, QString name, QJsonDocument parameters)
    : taskId(taskId), name(std::move(name)), parameters(std::move(parameters))
{
}

void NewStep::registerMetaTypes()
{
    qDBusRegisterMetaType<NewStep>();
    qDBusRegisterMetaType<NewStepList>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const NewStep &step)
{
    return argument << step.taskId << step.name << step.parameters;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, NewStep &step)
{
    return argument >> step.taskId >> step.name >> step.parameters;
}
