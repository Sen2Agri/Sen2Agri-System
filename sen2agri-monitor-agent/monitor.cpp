#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "monitor.hpp"
#include "stats.hpp"
#include "logger.hpp"

Monitor::Monitor(const Settings &settings, QObject *parent)
    : QObject(parent),
      serviceUrl(settings.serviceUrl),
      diskPath(defaultDiskPath),
      isConfigured()
{
    timer.setSingleShot(true);
    timer.setInterval(defaultScanInterval);

    Logger::info(QStringLiteral("Endpoint address is %1").arg(serviceUrl));

    connect(&timer, &QTimer::timeout, this, &Monitor::timerFired);

    getConfiguration();
}

void Monitor::getConfiguration()
{
    QNetworkRequest request(serviceUrl + QStringLiteral("GetMonitorAgentParameters"));

    auto reply = networkAccessManager.get(request);
    connect(reply, &QNetworkReply::finished, this, &Monitor::configurationRead);
}

void Monitor::sendStatistics()
{
    QNetworkRequest request(serviceUrl + QStringLiteral("SaveStatistics"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    const auto &json = getStatsJson(diskPath).toJson();

    auto reply = networkAccessManager.post(request, json);
    connect(reply, &QNetworkReply::finished, this, &Monitor::sendFinished);
}

void Monitor::timerFired()
{
    if (isConfigured) {
        sendStatistics();
    } else {
        getConfiguration();
    }
}

void Monitor::configurationRead()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    Q_ASSERT(reply);

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        Logger::error(QStringLiteral("Unable to read configuration: %1").arg(reply->errorString()));

        timer.start();
    } else {
        const auto &obj = QJsonDocument::fromJson(reply->readAll()).object();
        diskPath = obj[QStringLiteral("monitor-agent.disk-path")].toString();
        auto scanInterval = obj[QStringLiteral("monitor-agent.scan-interval")].toString().toInt();

        if (diskPath.isEmpty()) {
            Logger::error("Please configure monitor-agent.disk-path");
            diskPath = defaultDiskPath;
        }
        if (!scanInterval) {
            Logger::error("Please configure monitor-agent.scan-interval");
            scanInterval = defaultScanInterval;
        }
        isConfigured = true;

        timer.setInterval(scanInterval);

        Logger::info(QStringLiteral("Monitoring disk space on %1").arg(diskPath));
        Logger::info(QStringLiteral("Scan interval is %1 ms").arg(scanInterval));

        sendStatistics();
    }
}

void Monitor::sendFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    Q_ASSERT(reply);

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        Logger::error(
            QStringLiteral("Unable to submit the statistics: %1").arg(reply->errorString()));
    }

    timer.start();
}

QString Monitor::defaultDiskPath(QStringLiteral("/"));
