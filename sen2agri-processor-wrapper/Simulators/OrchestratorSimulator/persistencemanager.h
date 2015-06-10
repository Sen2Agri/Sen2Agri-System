#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QSettings>

#include "model.hpp"

class PersistenceManager : public QObject, protected QDBusContext
{
    Q_OBJECT

public:
    explicit PersistenceManager(QObject *parent = 0);

    void Initialize(QString &strCfgPath);
    QString & GetLastJobName();

signals:

public slots:
    ConfigurationSet GetConfigurationSet();
    ConfigurationParameterValueList GetConfigurationParameters(const QString &prefix);
    ConfigurationParameterValueList GetJobConfigurationParameters(int jobId, const QString &prefix);
    KeyedMessageList UpdateConfigurationParameters(const ConfigurationUpdateActionList &actions);
    KeyedMessageList
    UpdateJobConfigurationParameters(int jobId, const ConfigurationUpdateActionList &parameters);

    ProductToArchiveList GetProductsToArchive();
    KeyedMessageList UpdateProcessorStatus(const ProcessorStatusInfo &actions);

private:
    QSettings *m_pSettings;
    QString m_strLastJobName;
};
