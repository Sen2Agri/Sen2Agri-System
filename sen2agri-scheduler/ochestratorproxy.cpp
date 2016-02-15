#include "ochestratorproxy.hpp"
#include "orchestrator_interface.h"
#include "dbus_future_utils.hpp"

OchestratorProxy::OchestratorProxy()
    : orchestrator(OrgEsaSen2agriOrchestratorInterface::staticInterfaceName(),
                   QStringLiteral("/org/esa/sen2agri/orchestrator"),
                   QDBusConnection::systemBus())
{

}
OchestratorProxy::~OchestratorProxy()
{
}

JobDefinition OchestratorProxy::GetJobDefinition(const ProcessingRequest &request)
{
    JobDefinition jd;
    // TODO: add dbus call
    jd = WaitForResponseAndThrow(orchestrator.GetJobDefinition(request));
    return jd;
}

void OchestratorProxy::SubmitJob(const JobDefinition &job)
{
    orchestrator.SubmitJob(job);
}
