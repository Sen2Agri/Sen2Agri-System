#include "dashboardcontroller.h"

#include "persistencemanager_interface.h"

DashboardController::DashboardController()
{
}

void DashboardController::service(HttpRequest &request, HttpResponse &response)
{
    const auto &path = request.getPath();
    const auto &action = path.mid(path.indexOf('/', 1) + 1);

    if (action == "GetDashboardData") {
        getDashboardData(request, response);
    } else {
        response.setStatus(400, "Bad Request");
    }
}

void DashboardController::getDashboardData(HttpRequest &request, HttpResponse &response)
{
    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(), QStringLiteral("/"),
        QDBusConnection::systemBus());

    const auto &value = request.getParameter("since");
    const auto &since = QDate::fromString(value, Qt::ISODate);
    qDebug() << since;

    auto promise = persistenceManagerClient.GetDashboardData(since);
    promise.waitForFinished();

    if (promise.isError()) {
        response.setStatus(500, "Internal Server Error");

        qCritical() << promise.error().message();
    } else {
        response.setHeader("Content-Type", "application/json");
        response.write(promise.value().toUtf8(), true);
    }
}
