#include "dashboardcontroller.hpp"
#include "stopwatch.hpp"
#include "logger.hpp"
#include "dbus_future_utils.hpp"
#include "orchestrator_interface.h"

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
            if (action == "GetDashboardCurrentJobData") {
                getDashboardCurrentJobData(request, response);
            } else if (action == "GetDashboardServerResourceData") {
                getDashboardServerResourceData(request, response);
            } else if (action == "GetDashboardProcessorStatistics") {
                getDashboardProcessorStatistics(request, response);
            } else if (action == "GetDashboardProductAvailability") {
                getDashboardProductAvailability(request, response);
            } else if (action == "GetDashboardJobTimeline") {
                getDashboardJobTimeline(request, response);
            } else if (action == "GetDashboardProducts") {
                getDashboardProducts(request, response);
            } else if (action == "GetDashboardSites") {
                getDashboardSites(request, response);
            } else if (action == "GetDashboardSentinelTiles") {
                getDashboardSentinelTiles(request, response);
            } else if (action == "GetDashboardLandsatTiles") {
                getDashboardLandsatTiles(request, response);
            } else if (action == "GetDashboardProcessors") {
                getDashboardProcessors(request, response);
            } else {
                response.setStatus(400, "Bad Request");
            }
        } else if (method == "POST") {
            if (action == "CancelJob") {
                cancelJob(request, response);
            } else if (action == "PauseJob") {
                pauseJob(request, response);
            } else if (action == "ResumeJob") {
                resumeJob(request, response);
            } else if (action == "NotifyOrchestrator") {
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

void DashboardController::getDashboardCurrentJobData(const HttpRequest &, HttpResponse &response)
{
    const auto &data = persistenceManager.GetDashboardCurrentJobData();

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardServerResourceData(const HttpRequest &,
                                                         HttpResponse &response)
{
    const auto &data = persistenceManager.GetDashboardServerResourceData();

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardProcessorStatistics(const HttpRequest &,
                                                          HttpResponse &response)
{
    const auto &data = persistenceManager.GetDashboardProcessorStatistics();

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardProductAvailability(const HttpRequest &request,
                                                          HttpResponse &response)
{
    const auto &value = request.getParameter("since");
    const auto &since = QDateTime::fromString(value, Qt::ISODate);

    const auto &data = persistenceManager.GetDashboardProductAvailability(since);

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardJobTimeline(const HttpRequest &request,
                                                  HttpResponse &response)
{
    const auto &jobIdStr = request.getParameter("jobId");
    bool ok;
    auto jobId = jobIdStr.toInt(&ok);

    if (!ok) {
        Logger::error(QStringLiteral("Invalid jobId value: %1").arg(QString::fromUtf8(jobIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    const auto &data = persistenceManager.GetDashboardJobTimeline(jobId);

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardProducts(const HttpRequest &request, HttpResponse &response)
{
    bool ok;
    const auto &siteIdStr = request.getParameter("siteId");
    std::experimental::optional<int> siteId;

    if (!siteIdStr.isNull()) {
        siteId = siteIdStr.toInt(&ok);
        if (!ok) {
            Logger::error(
                QStringLiteral("Invalid siteId value: %1").arg(QString::fromUtf8(siteIdStr)));

            response.setStatus(400, "Bad Request");
            return;
        }
    }

    const auto &processorIdStr = request.getParameter("processorId");
    std::experimental::optional<int> processorId;

    if (!processorIdStr.isNull()) {
        processorId = processorIdStr.toInt(&ok);
        if (!ok) {
            Logger::error(QStringLiteral("Invalid processorId value: %1")
                              .arg(QString::fromUtf8(processorIdStr)));

            response.setStatus(400, "Bad Request");
            return;
        }
    }

    const auto &data = persistenceManager.GetDashboardProducts({ siteId, processorId });

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardSites(const HttpRequest &, HttpResponse &response)
{
    const auto &data = persistenceManager.GetDashboardSites();

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardSentinelTiles(const HttpRequest &request,
                                                    HttpResponse &response)
{
    const auto &siteIdStr = request.getParameter("siteId");
    bool ok;
    auto siteId = siteIdStr.toInt(&ok);

    if (!ok) {
        Logger::error(QStringLiteral("Invalid siteId value: %1").arg(QString::fromUtf8(siteIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    const auto &data = persistenceManager.GetDashboardSentinelTiles(siteId);

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardLandsatTiles(const HttpRequest &request,
                                                   HttpResponse &response)
{
    const auto &siteIdStr = request.getParameter("siteId");
    bool ok;
    auto siteId = siteIdStr.toInt(&ok);

    if (!ok) {
        Logger::error(QStringLiteral("Invalid siteId value: %1").arg(QString::fromUtf8(siteIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    const auto &data = persistenceManager.GetDashboardLandsatTiles(siteId);

    response.setHeader("Content-Type", "application/json");
    response.write(data.toUtf8(), true);
}

void DashboardController::getDashboardProcessors(const HttpRequest &, HttpResponse &response)
{
    const auto &data = persistenceManager.GetDashboardProcessors();

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

    persistenceManager.InsertEvent(JobCancelledEvent(jobId));

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

    persistenceManager.InsertEvent(JobPausedEvent(jobId));

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

    const auto &processorIdStr = request.getParameter("processorId");
    auto processorId = processorIdStr.toInt(&ok);
    if (!ok) {
        Logger::error(QStringLiteral("Invalid processorId value: %1")
                          .arg(QString::fromUtf8(processorIdStr)));

        response.setStatus(400, "Bad Request");
        return;
    }

    persistenceManager.InsertEvent(JobResumedEvent(jobId, processorId));

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
