#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "monitor.hpp"
#include "stats.hpp"
#include "logger.hpp"

Monitor::Monitor(const QSettings &settings, QObject *parent)
    : QObject(parent),
      serviceUrl(settings.value(QStringLiteral("ServiceUrl")).toString()),
      diskPath(defaultDiskPath),
      scanInterval(defaultScanInterval),
      isConfigured()
{
    timer.setSingleShot(true);
    timer.setInterval(scanInterval);

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
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        Logger::error(reply->errorString());

        timer.start();
    } else {
        const auto &obj = QJsonDocument::fromJson(reply->readAll()).object();
        diskPath = obj[QStringLiteral("monitor-agent.disk-path")].toString();
        scanInterval = obj[QStringLiteral("monitor-agent.scan-interval")].toString().toInt();

        if (diskPath.isEmpty()) {
            Logger::error("Please configure monitor-agent.disk-path");
            diskPath = defaultDiskPath;
        }
        if (!scanInterval) {
            Logger::error("Please configure monitor-agent.scan-interval");
            scanInterval = defaultScanInterval;
        }
        isConfigured = true;

        sendStatistics();
    }
}

void Monitor::sendFinished()
{
    auto reply = qobject_cast<QNetworkReply *>(sender());
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        Logger::error(reply->errorString());
    }

    timer.start();
}

QString Monitor::defaultDiskPath(QStringLiteral("/"));
