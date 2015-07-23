#ifndef ORCHESTRATORCLIENT_H
#define ORCHESTRATORCLIENT_H

#include "orchestrator_interface.h"

class OrchestratorClient
{
    OrgEsaSen2agriOrchestratorInterface orchestrator;

public:
    OrchestratorClient();

    void NotifyEventsAvailable();
};

#endif // ORCHESTRATORCLIENT_H
