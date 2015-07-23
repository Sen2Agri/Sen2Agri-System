#pragma once

#include <httpserver/httprequest.h>
#include <httpserver/httpresponse.h>
#include <httpserver/httprequesthandler.h>

class DashboardController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(DashboardController)

    void getDashboardData(const HttpRequest &request, HttpResponse &response);
    void getDashboardSystemOverviewData(const HttpRequest &request, HttpResponse &response);
    void getDashboardProcessorStatistics(const HttpRequest &request, HttpResponse &response);
    void getDashboardProductAvailability(const HttpRequest &request, HttpResponse &response);

    void cancelJob(const HttpRequest &request, HttpResponse &response);
    void pauseJob(const HttpRequest &request, HttpResponse &response);
    void resumeJob(const HttpRequest &request, HttpResponse &response);

    void notifyOrchestrator();

public:
    DashboardController();

    void service(HttpRequest &request, HttpResponse &response);
};
