#ifndef OCHESTRATORPROXY_H
#define OCHESTRATORPROXY_H

#include <QString>
#include <QVariant>
#include "model.hpp"
#include "orchestrator_interface.h"

class OchestratorProxy
{
    OrgEsaSen2agriOrchestratorInterface orchestrator;
public:
    OchestratorProxy();
    virtual ~OchestratorProxy();
    virtual JobDefinition GetJobDefinition(const ProcessingRequest &request);
    virtual void SubmitJob(const JobDefinition &job);
};

#endif // OCHESTRATORPROXY_H
