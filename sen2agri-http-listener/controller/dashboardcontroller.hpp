#pragma once

#include <httpserver/httprequest.h>
#include <httpserver/httprequesthandler.h>
#include <httpserver/httpresponse.h>

#include "persistencemanager.hpp"

class DashboardController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(DashboardController)

    PersistenceManagerDBProvider &persistenceManager;

    void notifyOrchestrator();

public:
    DashboardController(PersistenceManagerDBProvider &persistenceManager);

    void service(HttpRequest &request, HttpResponse &response);
};
