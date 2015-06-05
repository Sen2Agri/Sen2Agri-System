#pragma once

#include "sqldatabaseraii.hpp"
#include "dbprovider.hpp"
#include "model.hpp"
#include "settings.hpp"

class PersistenceManagerDBProvider
{
    DBProvider provider;

    SqlDatabaseRAII getDatabase() const;

public:
    PersistenceManagerDBProvider(const Settings &settings);

    ConfigurationSet GetConfigurationSet();

    ConfigurationParameterValueList GetConfigurationParameters(const QString &prefix);
    ConfigurationParameterValueList GetJobConfigurationParameters(int jobId, const QString &prefix);

    KeyedMessageList UpdateConfigurationParameters(const ConfigurationUpdateActionList &actions);
    KeyedMessageList UpdateJobConfigurationParameters(int jobId,
                                                      const ConfigurationUpdateActionList &actions);

    ProductToArchiveList GetProductsToArchive();
    void MarkProductsArchived(const ArchivedProductList &products);
};
