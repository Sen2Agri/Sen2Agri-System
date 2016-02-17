#pragma once

#include "taskloader.hpp"

class DatabaseTaskLoader : public TaskLoader
{
public:
    DatabaseTaskLoader();
    virtual ~DatabaseTaskLoader();
    virtual std::vector<ScheduledTask> LoadFromDatabase( );
    virtual void UpdateStatusinDatabase( const std::vector<ScheduledTask>& );
};
