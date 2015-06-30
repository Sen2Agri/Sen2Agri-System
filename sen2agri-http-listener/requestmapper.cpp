#include <httpserver/staticfilecontroller.h>

#include "requestmapper.h"
#include "controller/dashboardcontroller.h"

extern StaticFileController *staticFileController;

RequestMapper::RequestMapper(QObject *parent) : HttpRequestHandler(parent)
{
}

void RequestMapper::service(HttpRequest &request, HttpResponse &response)
{
    QByteArray path = request.getPath();

    if (path.startsWith("/dashboard/")) {
        DashboardController().service(request, response);
    } else {
        staticFileController->service(request, response);
    }
}
