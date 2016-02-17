#ifndef OCHESTRATORPROXY_H
#define OCHESTRATORPROXY_H

#include "model.hpp"

class OrchestratorProxy
{
public:
    virtual ~OrchestratorProxy();
    virtual JobDefinition GetJobDefinition(const ProcessingRequest &request) = 0;
    virtual void SubmitJob(const JobDefinition &job) = 0;
};

#endif // OCHESTRATORPROXY_H
