#pragma once

#include <QString>
#include <QDBusArgument>
#include <QMetaType>
#include <QDateTime>
#include <QJsonDocument>

#include "optional.hpp"

void registerMetaTypes();

class ConfigurationParameterInfo
{
public:
    QString key;
    int categoryId;
    QString friendlyName;
    QString dataType;
    bool isAdvanced;

    ConfigurationParameterInfo();
    ConfigurationParameterInfo(
        QString key, int categoryId, QString friendlyName, QString dataType, bool isAdvanced);

    static void registerMetaTypes();
};

typedef QList<ConfigurationParameterInfo> ConfigurationParameterInfoList;

Q_DECLARE_METATYPE(ConfigurationParameterInfo)
Q_DECLARE_METATYPE(ConfigurationParameterInfoList)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameterInfo &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterInfo &parameter);

class ConfigurationParameterValue
{
public:
    QString key;
    std::experimental::optional<int> siteId;
    QString value;

    ConfigurationParameterValue();
    ConfigurationParameterValue(QString key,
                                std::experimental::optional<int> siteId,
                                QString value);

    static void registerMetaTypes();
};

typedef QList<ConfigurationParameterValue> ConfigurationParameterValueList;

Q_DECLARE_METATYPE(ConfigurationParameterValue)
Q_DECLARE_METATYPE(ConfigurationParameterValueList)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationParameterValue &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument,
                                ConfigurationParameterValue &parameter);

class ConfigurationCategory
{
public:
    int categoryId;
    QString name;

    ConfigurationCategory();
    ConfigurationCategory(int categoryId, QString name);

    static void registerMetaTypes();
};

typedef QList<ConfigurationCategory> ConfigurationCategoryList;

Q_DECLARE_METATYPE(ConfigurationCategory)
Q_DECLARE_METATYPE(ConfigurationCategoryList)

class Site
{
public:
    int siteId;
    QString name;

    Site();
    Site(int siteId, QString name);

    static void registerMetaTypes();
};

typedef QList<Site> SiteList;

Q_DECLARE_METATYPE(Site)
Q_DECLARE_METATYPE(SiteList)

QDBusArgument &operator<<(QDBusArgument &argument, const Site &site);
const QDBusArgument &operator>>(const QDBusArgument &argument, Site &site);

class ConfigurationSet
{
public:
    ConfigurationCategoryList categories;
    ConfigurationParameterInfoList parameterInfo;
    ConfigurationParameterValueList parameterValues;
    SiteList sites;
    bool isAdmin;

    ConfigurationSet();
    ConfigurationSet(ConfigurationCategoryList categories,
                     ConfigurationParameterInfoList parameterInfo,
                     ConfigurationParameterValueList parameterValues,
                     SiteList sites,
                     bool isAdmin);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(ConfigurationSet)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationSet &parameter);
const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationSet &parameter);

class ConfigurationUpdateAction
{
public:
    QString key;
    std::experimental::optional<int> siteId;
    std::experimental::optional<QString> value;

    ConfigurationUpdateAction();
    ConfigurationUpdateAction(QString key,
                              std::experimental::optional<int> siteId,
                              std::experimental::optional<QString> value);

    static void registerMetaTypes();
};

typedef QList<ConfigurationUpdateAction> ConfigurationUpdateActionList;

Q_DECLARE_METATYPE(ConfigurationUpdateAction)
Q_DECLARE_METATYPE(ConfigurationUpdateActionList)

QDBusArgument &operator<<(QDBusArgument &argument, const ConfigurationUpdateAction &action);
const QDBusArgument &operator>>(const QDBusArgument &argument, ConfigurationUpdateAction &action);

class KeyedMessage
{
public:
    QString key;
    QString text;

    KeyedMessage();
    KeyedMessage(QString key, QString text);

    static void registerMetaTypes();
};

typedef QList<KeyedMessage> KeyedMessageList;

Q_DECLARE_METATYPE(KeyedMessage)
Q_DECLARE_METATYPE(KeyedMessageList)

QDBusArgument &operator<<(QDBusArgument &argument, const KeyedMessage &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, KeyedMessage &message);

class Product
{
public:
    int productId;
    int processorId;
    int productTypeId;
    int siteId;
    QString fullPath;
    QDateTime created;

    Product();
    Product(int productId,
            int processorId,
            int productTypeId,
            int siteId,
            QString fullPath,
            QDateTime created);

    static void registerMetaTypes();
};

typedef QList<Product> ProductList;

Q_DECLARE_METATYPE(Product)
Q_DECLARE_METATYPE(ProductList)

QDBusArgument &operator<<(QDBusArgument &argument, const Product &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, Product &message);

class ProductToArchive
{
public:
    int productId;
    QString currentPath;
    QString archivePath;

    ProductToArchive();
    ProductToArchive(int productId, QString currentPath, QString archivePath);

    static void registerMetaTypes();
};

typedef QList<ProductToArchive> ProductToArchiveList;

Q_DECLARE_METATYPE(ProductToArchive)
Q_DECLARE_METATYPE(ProductToArchiveList)

QDBusArgument &operator<<(QDBusArgument &argument, const ProductToArchive &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, ProductToArchive &message);

class ArchivedProduct
{
public:
    int productId;
    QString archivePath;

    ArchivedProduct();
    ArchivedProduct(int productId, QString archivePath);

    static void registerMetaTypes();
};

typedef QList<ArchivedProduct> ArchivedProductList;

Q_DECLARE_METATYPE(ArchivedProduct)
Q_DECLARE_METATYPE(ArchivedProductList)

QDBusArgument &operator<<(QDBusArgument &argument, const ArchivedProduct &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, ArchivedProduct &message);

enum class JobStartType { Triggered = 1, Requested = 2, Scheduled = 3 };

Q_DECLARE_METATYPE(JobStartType)

QDBusArgument &operator<<(QDBusArgument &argument, JobStartType startType);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobStartType &startType);

Q_DECLARE_METATYPE(QJsonDocument)

QDBusArgument &operator<<(QDBusArgument &argument, const QJsonDocument &document);
const QDBusArgument &operator>>(const QDBusArgument &argument, QJsonDocument &document);

class NewJob
{
public:
    QString name;
    QString description;
    int processorId;
    int siteId;
    JobStartType startType;
    QJsonDocument parameters;

    NewJob();
    NewJob(QString name,
           QString description,
           int processorId,
           int siteId,
           JobStartType startType,
           QJsonDocument parameters);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(NewJob)

QDBusArgument &operator<<(QDBusArgument &argument, const NewJob &job);
const QDBusArgument &operator>>(const QDBusArgument &argument, NewJob &job);

class NewTask
{
public:
    int jobId;
    int moduleId;
    QJsonDocument parameters;

    NewTask();
    NewTask(int jobId, int moduleId, QJsonDocument parameters);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(NewTask)

QDBusArgument &operator<<(QDBusArgument &argument, const NewTask &task);
const QDBusArgument &operator>>(const QDBusArgument &argument, NewTask &task);

class NewStep
{
public:
    int taskId;
    QString name;
    QJsonDocument parameters;

    NewStep();
    NewStep(int taskId, QString name, QJsonDocument parameters);

    static void registerMetaTypes();
};

typedef QList<NewStep> NewStepList;

Q_DECLARE_METATYPE(NewStep)
Q_DECLARE_METATYPE(NewStepList)

QDBusArgument &operator<<(QDBusArgument &argument, const NewStep &step);
const QDBusArgument &operator>>(const QDBusArgument &argument, NewStep &step);

enum class ExecutionStatus {
    Submitted,
    PendingStart,
    NeedsInput,
    Running,
    Paused,
    Finished,
    Cancelled,
    Error
};

Q_DECLARE_METATYPE(ExecutionStatus)

QDBusArgument &operator<<(QDBusArgument &argument, ExecutionStatus status);
const QDBusArgument &operator>>(const QDBusArgument &argument, ExecutionStatus &status);

class ExecutionStatistics
{
public:
    QString node;
    int32_t exitCode;
    int64_t userCpuMs;
    int64_t systemCpuMs;
    int64_t durationMs;
    int32_t maxRssKb;
    int32_t maxVmSizeKb;
    int64_t diskReadBytes;
    int64_t diskWriteBytes;

    ExecutionStatistics();
    ExecutionStatistics(QString node,
                        int32_t exitCode,
                        int64_t userCpuMs,
                        int64_t systemCpuMs,
                        int64_t durationMs,
                        int32_t maxRssKb,
                        int32_t maxVmSizeKb,
                        int64_t diskReadBytes,
                        int64_t diskWriteBytes);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(ExecutionStatistics)

QDBusArgument &operator<<(QDBusArgument &argument, const ExecutionStatistics &statistics);
const QDBusArgument &operator>>(const QDBusArgument &argument, ExecutionStatistics &statistics);

enum class EventType {
    TaskFinished,
    ProductAvailable,
    JobCancelled,
    JobPaused,
    JobResumed,
    JobSubmitted,
    StepFailed
};

Q_DECLARE_METATYPE(EventType)

QDBusArgument &operator<<(QDBusArgument &argument, EventType event);
const QDBusArgument &operator>>(const QDBusArgument &argument, EventType &event);

class TaskFinishedEvent
{
public:
    int taskId;

    TaskFinishedEvent();
    TaskFinishedEvent(int taskId);

    QJsonDocument toJson() const;

    static TaskFinishedEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(TaskFinishedEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const TaskFinishedEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, TaskFinishedEvent &event);

class ProductAvailableEvent
{
public:
    int productId;

    ProductAvailableEvent();
    ProductAvailableEvent(int productId);

    QJsonDocument toJson() const;

    static ProductAvailableEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(ProductAvailableEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const ProductAvailableEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, ProductAvailableEvent &event);

class JobCancelledEvent
{
public:
    int jobId;

    JobCancelledEvent();
    JobCancelledEvent(int jobId);

    QJsonDocument toJson() const;

    static JobCancelledEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(JobCancelledEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const JobCancelledEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobCancelledEvent &event);

class JobPausedEvent
{
public:
    int jobId;

    JobPausedEvent();
    JobPausedEvent(int jobId);

    QJsonDocument toJson() const;

    static JobPausedEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(JobPausedEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const JobPausedEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobPausedEvent &event);

class JobResumedEvent
{
public:
    int jobId;

    JobResumedEvent();
    JobResumedEvent(int jobId);

    QJsonDocument toJson() const;

    static JobResumedEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(JobResumedEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const JobResumedEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobResumedEvent &event);

class JobSubmittedEvent
{
public:
    int jobId;

    JobSubmittedEvent();
    JobSubmittedEvent(int jobId);

    QJsonDocument toJson() const;

    static JobSubmittedEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(JobSubmittedEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const JobSubmittedEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobSubmittedEvent &event);

class StepFailedEvent
{
public:
    int taskId;
    QString stepName;

    StepFailedEvent();
    StepFailedEvent(int taskId, QString stepName);

    QJsonDocument toJson() const;

    static StepFailedEvent fromJson(const QJsonDocument &json);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(StepFailedEvent)

QDBusArgument &operator<<(QDBusArgument &argument, const StepFailedEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, StepFailedEvent &event);

class UnprocessedEvent
{
public:
    int eventId;
    EventType type;
    QJsonDocument data;
    QDateTime submittedTime;
    std::experimental::optional<QDateTime> processingStartedTime;

    UnprocessedEvent();
    UnprocessedEvent(int eventId,
                     EventType type,
                     QJsonDocument data,
                     QDateTime submittedTime,
                     std::experimental::optional<QDateTime> processingStartedTime);

    static void registerMetaTypes();
};

typedef QList<UnprocessedEvent> UnprocessedEventList;

Q_DECLARE_METATYPE(UnprocessedEvent)
Q_DECLARE_METATYPE(UnprocessedEventList)

QDBusArgument &operator<<(QDBusArgument &argument, const UnprocessedEvent &event);
const QDBusArgument &operator>>(const QDBusArgument &argument, UnprocessedEvent &event);

class NodeStatistics
{
public:
    QString node;
    int32_t freeRamKb;
    int64_t freeDiskBytes;

    NodeStatistics();
    NodeStatistics(QString node, int32_t freeRamKb, int64_t freeDiskBytes);

    static void registerMetaTypes();
};

typedef QList<NodeStatistics> NodeStatisticsList;

Q_DECLARE_METATYPE(NodeStatistics)
Q_DECLARE_METATYPE(NodeStatisticsList)

QDBusArgument &operator<<(QDBusArgument &argument, NodeStatistics statistics);
const QDBusArgument &operator>>(const QDBusArgument &argument, NodeStatistics &statistics);
