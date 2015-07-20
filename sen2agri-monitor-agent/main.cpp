#include <QCoreApplication>
#include <QJsonDocument>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>

#include "logger.hpp"
#include "monitor.hpp"

int main(int argc, char *argv[])
{
    Logger::installMessageHandler();

    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("sen2agri-monitor-agent"));

    QSettings settings;
    settings.setValue(QStringLiteral("serviceUrl"),
                      QStringLiteral("http://127.0.0.1:8080/statistics/"));

    Monitor monitor(settings);
    Q_UNUSED(monitor);

    return app.exec();
}
