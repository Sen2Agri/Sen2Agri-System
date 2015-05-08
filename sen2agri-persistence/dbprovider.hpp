#pragma once

#include <QString>
#include <QtSql>

#include "configurationparameter.hpp"

class DBProvider
{
    QSqlDatabase db;

    QSqlQuery createQuery();
    QSqlQuery prepareQuery(const QString &query);

public:
    DBProvider();

    ConfigurationParameterList GetConfigurationParameters(const QString &prefix);
};
