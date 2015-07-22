#pragma once

#include <httpserver/httprequest.h>
#include <httpserver/httpresponse.h>
#include <httpserver/httprequesthandler.h>

class StatisticsController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(StatisticsController)

    void saveDashboardData(const HttpRequest &request, HttpResponse &response);
    void getMonitorAgentParameters(const HttpRequest &request, HttpResponse &response);

public:
    StatisticsController();

    void service(HttpRequest &request, HttpResponse &response);
};
