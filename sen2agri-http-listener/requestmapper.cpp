#include <QByteArray>

#include "stopwatch.hpp"
#include "requestmapper.hpp"
#include "controller/dashboardcontroller.hpp"

RequestMapper::RequestMapper(StaticFileController &staticFileController, QObject *parent)
    : HttpRequestHandler(parent), staticFileController(staticFileController)
{
}

void RequestMapper::service(HttpRequest &request, HttpResponse &response)
{
    START_STOPWATCH("RequestMapper::service");

    if (request.getPath().startsWith("/dashboard/")) {
        DashboardController().service(request, response);
    } else {
        staticFileController.service(request, response);
    }
}
