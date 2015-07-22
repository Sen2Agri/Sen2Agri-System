#include <QByteArray>

#include "stopwatch.hpp"
#include "requestmapper.hpp"
#include "controller/dashboardcontroller.hpp"
#include "controller/statisticscontroller.hpp"

RequestMapper::RequestMapper(StaticFileController &staticFileController, QObject *parent)
    : HttpRequestHandler(parent), staticFileController(staticFileController)
{
}

void RequestMapper::service(HttpRequest &request, HttpResponse &response)
{
    START_STOPWATCH("RequestMapper::service");

    const auto &path = request.getPath();
    if (path.startsWith("/dashboard/")) {
        DashboardController().service(request, response);
    } else if (path.startsWith("/statistics/")) {
        StatisticsController().service(request, response);
    } else {
        staticFileController.service(request, response);
    }
}
