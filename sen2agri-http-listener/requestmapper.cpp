#include <QByteArray>

#include "stopwatch.hpp"
#include "requestmapper.hpp"
#include "controller/dashboardcontroller.hpp"
#include "controller/statisticscontroller.hpp"

RequestMapper::RequestMapper(StaticFileController &staticFileController,
                             PersistenceManagerDBProvider &persistenceManager,
                             QObject *parent)
    : HttpRequestHandler(parent),
      staticFileController(staticFileController),
      persistenceManager(persistenceManager)
{
}

void RequestMapper::service(HttpRequest &request, HttpResponse &response)
{
    START_STOPWATCH("RequestMapper::service");

    const auto &path = request.getPath();
    if (path.startsWith("/dashboard/")) {
        DashboardController(persistenceManager).service(request, response);
    } else if (path.startsWith("/statistics/")) {
        StatisticsController(persistenceManager).service(request, response);
    } else {
        staticFileController.service(request, response);
    }
}
