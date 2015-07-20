#include <QCoreApplication>
#include <QJsonDocument>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QFileInfo>

#include "logger.hpp"
#include "configuration.hpp"

#include "monitor.hpp"

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QCoreApplication app(argc, argv);

        const auto &configFile = getConfigurationFile(app);

        QSettings settings;
        settings.setValue(QStringLiteral("serviceUrl"),
                          QStringLiteral("http://127.0.0.1:8080/statistics/"));

        Monitor monitor(settings);
        Q_UNUSED(monitor);

        return app.exec();
    } catch (const std::exception &e) {
        Logger::fatal(e.what());

        return EXIT_FAILURE;
    }
}
