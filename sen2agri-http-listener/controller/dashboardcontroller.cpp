#include "dashboardcontroller.hpp"
#include "dbus_future_utils.hpp"
#include "logger.hpp"
#include "orchestrator_interface.h"
#include "stopwatch.hpp"

DashboardController::DashboardController(PersistenceManagerDBProvider &persistenceManager)
    : persistenceManager(persistenceManager)
{
}

void DashboardController::service(HttpRequest &request, HttpResponse &response)
{
    Stopwatch sw(__func__);
    Q_UNUSED(sw);

    try {
        const auto &path = request.getPath();
        const auto &method = request.getMethod();
        const auto &action = path.mid(path.indexOf('/', 1) + 1);

        response.setHeader("Access-Control-Allow-Origin", "*");

        if (method == "GET") {
            response.setStatus(400, "Bad Request");
        } else if (method == "POST") {
            if (action == "NotifyOrchestrator") {
                notifyOrchestrator();
            } else {
                response.setStatus(400, "Bad Request");
            }
        }
    } catch (const std::exception &e) {
        response.setStatus(500, "Internal Server Error");

        Logger::error(e.what());
    }
}

void DashboardController::notifyOrchestrator()
{
    OrgEsaSen2agriOrchestratorInterface orchestratorClient(
        OrgEsaSen2agriOrchestratorInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/orchestrator"), QDBusConnection::systemBus());

    auto promise = orchestratorClient.NotifyEventsAvailable();
    promise.waitForFinished();
    if (promise.isError()) {
        Logger::error(QStringLiteral("Unable to notify the orchestrator about new events: %1")
                          .arg(promise.error().message()));
    }
}
