#include "dashboardcontroller.hpp"
#include "stopwatch.hpp"
#include "logger.hpp"
#include "dbus_future_utils.hpp"
#include "persistencemanager_interface.h"
#include "orchestrator_interface.h"

DashboardController::DashboardController()
{
}

void DashboardController::service(HttpRequest &request, HttpResponse &response)
{
    Stopwatch sw(__func__);
    Q_UNUSED(sw);

    try {
        const auto &path = request.getPath();
        const auto &action = path.mid(path.indexOf('/', 1) + 1);

        if (action == "GetDashboardData") {
            getDashboardData(request, response);
        } else if (action == "CancelJob") {
            cancelJob(request, response);
        } else if (action == "PauseJob") {
            pauseJob(request, response);
        } else if (action == "ResumeJob") {
            resumeJob(request, response);
        } else {
            response.setStatus(400, "Bad Request");
        }
    } catch (const std::exception &e) {
        response.setStatus(500, "Internal Server Error");

        Logger::error(e.what());
    }
}

void DashboardController::getDashboardData(const HttpRequest &request, HttpResponse &response)
{
    const auto &value = request.getParameter("since");
    const auto &since = QDate::fromString(value, Qt::ISODate);

    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/persistenceManager"), QDBusConnection::systemBus());

    const auto &data = WaitForResponseAndThrow(persistenceManagerClient.GetDashboardData(since));

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::cancelJob(const HttpRequest &request, HttpResponse &response)
{
    const auto &jobIdStr = request.getParameter("jobId");
    bool ok;
    auto jobId = jobIdStr.toInt(&ok);

    if (!ok) {
        Logger::error(QStringLiteral("Invalid jobId value: %1").arg(QString::fromUtf8(jobIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/persistenceManager"), QDBusConnection::systemBus());

    WaitForResponseAndThrow(persistenceManagerClient.InsertJobCancelledEvent({ jobId }));

    notifyOrchestrator();
}

void DashboardController::pauseJob(const HttpRequest &request, HttpResponse &response)
{
    const auto &jobIdStr = request.getParameter("jobId");
    bool ok;
    auto jobId = jobIdStr.toInt(&ok);

    if (!ok) {
        Logger::error(QStringLiteral("Invalid jobId value: %1").arg(QString::fromUtf8(jobIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/persistenceManager"), QDBusConnection::systemBus());

    WaitForResponseAndThrow(persistenceManagerClient.InsertJobPausedEvent({ jobId }));

    notifyOrchestrator();
}

void DashboardController::resumeJob(const HttpRequest &request, HttpResponse &response)
{
    const auto &jobIdStr = request.getParameter("jobId");
    bool ok;
    auto jobId = jobIdStr.toInt(&ok);

    if (!ok) {
        Logger::error(QStringLiteral("Invalid jobId value: %1").arg(QString::fromUtf8(jobIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    OrgEsaSen2agriPersistenceManagerInterface persistenceManagerClient(
        OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
        QStringLiteral("/org/esa/sen2agri/persistenceManager"), QDBusConnection::systemBus());

    WaitForResponseAndThrow(persistenceManagerClient.InsertJobCancelledEvent({ jobId }));

    notifyOrchestrator();
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
