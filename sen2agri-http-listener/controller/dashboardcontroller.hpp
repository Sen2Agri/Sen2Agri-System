#pragma once

#include <httpserver/httprequest.h>
#include <httpserver/httpresponse.h>
#include <httpserver/httprequesthandler.h>

class DashboardController : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(DashboardController)

    void getDashboardData(HttpRequest &request, HttpResponse &response);

public:
    DashboardController();

    void service(HttpRequest &request, HttpResponse &response);
};
