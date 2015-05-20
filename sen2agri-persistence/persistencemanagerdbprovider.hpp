#pragma once

#include "qsqldatabaseraii.hpp"
#include "dbprovider.hpp"
#include "configurationparameter.hpp"
#include "keyedmessage.hpp"
#include "settings.hpp"

class PersistenceManagerDBProvider
{
    DBProvider provider;

    SqlDatabaseRAII getDatabase() const;

public:
    PersistenceManagerDBProvider(const Settings &settings);

    ConfigurationSet GetConfigurationSet();
    ConfigurationParameterValueList GetConfigurationParameters(const QString &prefix);
    KeyedMessageList
    UpdateConfigurationParameters(const ConfigurationParameterValueList &parameters);
};
