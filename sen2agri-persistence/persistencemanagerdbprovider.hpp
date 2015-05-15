#pragma once

#include "qsqldatabaseraii.hpp"
#include "dbprovider.hpp"
#include "configurationparameter.hpp"
#include "keyedmessage.hpp"

class PersistenceManagerDBProvider
{
    DBProvider provider;

    SqlDatabaseRAII getDatabase() const;

public:
    ConfigurationSet GetConfigurationSet();
    ConfigurationParameterValueList GetConfigurationParameters(const QString &prefix);
    KeyedMessageList
    UpdateConfigurationParameters(const ConfigurationParameterValueList &parameters);
};
