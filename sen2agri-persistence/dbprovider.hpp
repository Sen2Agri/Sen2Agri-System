#pragma once

#include <utility>

#include <QString>
#include <QtSql>

#include "qsqldatabaseraii.hpp"
#include "configurationparameter.hpp"

class DBProvider
{
    QSqlDatabase db;

    QSqlDatabaseRAII getDatabase();

public:
    DBProvider();

    ConfigurationParameterList GetConfigurationParameters(const QString &prefix);
};
