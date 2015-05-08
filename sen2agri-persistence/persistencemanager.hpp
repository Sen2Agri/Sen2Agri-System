#pragma once

#include <QObject>
#include <QDBusAbstractAdaptor>

#include "dbprovider.hpp"
#include "configurationparameter.hpp"

class PersistenceManager : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.esa.sen2agri.persistenceManager")

    DBProvider dbProvider;

public:
    explicit PersistenceManager(QObject *parent = 0);

    static void registerMetaTypes();

signals:

public slots:
    ConfigurationParameterList GetConfigurationParameters(const QString &prefix);
};
