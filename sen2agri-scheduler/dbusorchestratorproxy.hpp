#pragma once

#include <memory>
#include "model.hpp"

#include "orchestratorproxy.hpp"

class OrgEsaSen2agriOrchestratorInterface;

class DBusOrchestratorProxy : public OrchestratorProxy
{
    std::unique_ptr<OrgEsaSen2agriOrchestratorInterface> orchestrator;

public:
    DBusOrchestratorProxy();
    virtual ~DBusOrchestratorProxy();
    virtual JobDefinition GetJobDefinition(const ProcessingRequest &request);
    virtual void SubmitJob(const JobDefinition &job);
};
