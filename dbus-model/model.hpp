#pragma once

#include <QString>
#include <QDBusArgument>
#include <QMetaType>
#include <QDateTime>

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

Q_DECLARE_METATYPE(ConfigurationCategory);
Q_DECLARE_METATYPE(ConfigurationCategoryList);

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

Q_DECLARE_METATYPE(Site);
Q_DECLARE_METATYPE(SiteList);

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

Q_DECLARE_METATYPE(ConfigurationSet);

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

Q_DECLARE_METATYPE(ConfigurationUpdateAction);
Q_DECLARE_METATYPE(ConfigurationUpdateActionList);

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

Q_DECLARE_METATYPE(ArchivedProduct);
Q_DECLARE_METATYPE(ArchivedProductList);

QDBusArgument &operator<<(QDBusArgument &argument, const ArchivedProduct &message);
const QDBusArgument &operator>>(const QDBusArgument &argument, ArchivedProduct &message);

enum class JobStartType { Triggered = 1, Requested = 2, Scheduled = 3 };

Q_DECLARE_METATYPE(JobStartType);

QDBusArgument &operator<<(QDBusArgument &argument, JobStartType startType);
const QDBusArgument &operator>>(const QDBusArgument &argument, JobStartType &startType);

class NewJob
{
public:
    int processorId;
    int productId;
    int siteId;
    JobStartType startType;
    QString inputPath;
    QString outputPath;
    int stepsTotal;

    NewJob();
    NewJob(int processorId,
           int productId,
           int siteId,
           JobStartType startType,
           QString inputPath,
           QString outputPath,
           int stepsTotal);

    static void registerMetaTypes();
};

Q_DECLARE_METATYPE(NewJob)

QDBusArgument &operator<<(QDBusArgument &argument, const NewJob &job);
const QDBusArgument &operator>>(const QDBusArgument &argument, NewJob &job);

class NewTask
{
public:
    int jobId;
    int productId;
    QString inputPath;
    QString outputPath;

    NewTask();
    NewTask(int jobId, int productId, QString inputPath, QString outputPath);

    static void registerMetaTypes();
};

typedef QList<NewTask> NewTaskList;

Q_DECLARE_METATYPE(NewTask);
Q_DECLARE_METATYPE(NewTaskList);

QDBusArgument &operator<<(QDBusArgument &argument, const NewTask &task);
const QDBusArgument &operator>>(const QDBusArgument &argument, NewTask &task);

class NewStep
{
public:
    int taskId;
    QString inputPath;
    QString outputPath;

    NewStep();
    NewStep(int taskId, QString inputPath, QString outputPath);

    static void registerMetaTypes();
};

typedef QList<NewStep> NewStepList;

Q_DECLARE_METATYPE(NewStep);
Q_DECLARE_METATYPE(NewStepList);

QDBusArgument &operator<<(QDBusArgument &argument, const NewStep &step);
const QDBusArgument &operator>>(const QDBusArgument &argument, NewStep &step);
