#include "dbusorchestratorproxy.hpp"
#include "orchestrator_interface.h"
#include "make_unique.hpp"
#include "dbus_future_utils.hpp"

DBusOrchestratorProxy::DBusOrchestratorProxy()
    : orchestrator(std::make_unique<OrgEsaSen2agriOrchestratorInterface>(
          OrgEsaSen2agriOrchestratorInterface::staticInterfaceName(),
          QStringLiteral("/org/esa/sen2agri/orchestrator"),
          QDBusConnection::systemBus()))
{
}

DBusOrchestratorProxy::~DBusOrchestratorProxy()
{
}

JobDefinition DBusOrchestratorProxy::GetJobDefinition(const ProcessingRequest &request)
{
    return WaitForResponseAndThrow(orchestrator->GetJobDefinition(request));
}

void DBusOrchestratorProxy::SubmitJob(const JobDefinition &job)
{
    orchestrator->SubmitJob(job);
}
