#include <QJsonDocument>

#include "statisticscontroller.hpp"
#include "stopwatch.hpp"
#include "logger.hpp"
#include "dbus_future_utils.hpp"
#include "persistencemanager_interface.h"
#include "orchestrator_interface.h"

StatisticsController::StatisticsController()
{
}

void StatisticsController::service(HttpRequest &request, HttpResponse &response)
{
    Stopwatch sw(__func__);
    Q_UNUSED(sw);

    try {
        const auto &path = request.getPath();
        const auto &action = path.mid(path.indexOf('/', 1) + 1);

        if (action == "SaveStatistics") {
            saveDashboardData(request, response);
        } else if (action == "GetMonitorAgentParameters") {
            getMonitorAgentParameters(request, response);
        } else {
            response.setStatus(400, "Bad Request");
        }
    } catch (const std::exception &e) {
        response.setStatus(500, "Internal Server Error");

        Logger::error(e.what());
    }
}

void StatisticsController::saveDashboardData(const HttpRequest &request, HttpResponse &)
{
    const auto &obj = QJsonDocument::fromJson(request.getBody()).object();

    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/persistenceManager"), QDBusConnection::systemBus());

    WaitForResponseAndThrow(persistenceManagerClient.InsertNodeStatistics(
        { obj[QStringLiteral("hostname")].toString(),
          static_cast<int64_t>(obj[QStringLiteral("mem_total_kb")].toDouble()),
          static_cast<int64_t>(obj[QStringLiteral("mem_used_kb")].toDouble()),
          static_cast<int64_t>(obj[QStringLiteral("swap_total_kb")].toDouble()),
          static_cast<int64_t>(obj[QStringLiteral("swap_used_kb")].toDouble()),
          obj[QStringLiteral("load_avg_1m")].toDouble(),
          obj[QStringLiteral("load_avg_5m")].toDouble(),
          obj[QStringLiteral("load_avg_15m")].toDouble(),
          static_cast<int64_t>(obj[QStringLiteral("disk_total_bytes")].toDouble()),
          static_cast<int64_t>(obj[QStringLiteral("disk_used_bytes")].toDouble()) }));
}

void StatisticsController::getMonitorAgentParameters(const HttpRequest &, HttpResponse &response)
{
    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/persistenceManager"), QDBusConnection::systemBus());

    auto parameters = WaitForResponseAndThrow(
        persistenceManagerClient.GetConfigurationParameters(QStringLiteral("monitor-agent.")));

    QJsonObject obj;
    for (const auto &p : parameters) {
        obj[p.key] = p.value;
    }

    response.write(QJsonDocument(obj).toJson());
}
