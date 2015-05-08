#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>
#include <QDBusContext>

#include "dbprovider.hpp"
#include "configurationparameter.hpp"

class PersistenceManager : public QObject, protected QDBusContext
{
    Q_OBJECT

    DBProvider dbProvider;

public:
    explicit PersistenceManager(QObject *parent = 0);

    static void registerMetaTypes();

signals:

public slots:
    ConfigurationParameterList GetConfigurationParameters(const QString &prefix);
};
