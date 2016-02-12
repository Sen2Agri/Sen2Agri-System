#include "ochestratorproxy.hpp"

OchestratorProxy::OchestratorProxy()
{

}
OchestratorProxy::~OchestratorProxy()
{
}

JobDefinition OchestratorProxy::GetJobDefinition(const ProcessingRequest &request)
{
    JobDefinition jd;
    // TODO: add dbus call
    return jd;
}

void OchestratorProxy::SubmitJob(const JobDefinition &job)
{
    // TODO: add dbus call
}
