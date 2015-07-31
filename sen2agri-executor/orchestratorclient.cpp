#include "orchestratorclient.h"

OrchestratorClient::OrchestratorClient()
    : orchestrator(OrgEsaSen2agriOrchestratorInterface::staticInterfaceName(),
                   QStringLiteral("/org/esa/sen2agri/orchestrator"),
                   QDBusConnection::systemBus())
{
}

void OrchestratorClient::NotifyEventsAvailable()
{
    orchestrator.NotifyEventsAvailable();
}
