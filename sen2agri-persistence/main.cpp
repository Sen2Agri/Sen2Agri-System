#include <stdexcept>

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QProcessEnvironment>
#include <QDBusConnection>
#include <QDebug>
#include <QDBusError>
#include <QFileInfo>

#include "persistencemanager.hpp"
#include "persistencemanager_adaptor.h"
#include "settings.hpp"
#include "logger.hpp"

int main(int argc, char *argv[])
{
    try {
        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(QStringLiteral("sen2agri-persistence"));

        QCommandLineParser parser;
        QCommandLineOption configFileOption(QStringLiteral("f"),
                                            QStringLiteral("Use this config file"),
                                            QStringLiteral("config file"));
        parser.addOption(configFileOption);
        parser.addHelpOption();
        parser.process(app);

        QString configFile;
        if (parser.isSet(configFileOption)) {
            configFile = parser.value(configFileOption);
        } else {
            configFile = QProcessEnvironment::systemEnvironment().value(
                QStringLiteral("SEN2AGRI_CONFIG_DIR"), QStringLiteral("/etc/sen2agri"));
            configFile += QStringLiteral("/sen2agri-persistence.conf");
        }

        QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, configFile);
        if (!QFileInfo::exists(configFile)) {
            throw std::runtime_error(
                QStringLiteral("Configuration file %1 does not exist, exiting.")
                    .arg(configFile)
                    .toStdString());
        }

        const auto &settings = Settings::readSettings(configFile);

        PersistenceManager::registerMetaTypes();

        QDBusConnection connection = QDBusConnection::systemBus();
        PersistenceManager persistenceManager(connection, settings);

        new PersistenceManagerAdaptor(&persistenceManager);

        if (!connection.registerObject(QStringLiteral("/"), &persistenceManager)) {
            throw std::runtime_error(
                QStringLiteral("Error registering the object with D-Bus: %1, exiting.")
                    .arg(connection.lastError().message())
                    .toStdString());
        }

        if (!connection.registerService(QStringLiteral("org.esa.sen2agri.persistenceManager"))) {
            throw std::runtime_error(
                QStringLiteral("Error registering the service with D-Bus: %1, exiting.")
                    .arg(connection.lastError().message())
                    .toStdString());
        }

        return app.exec();
    } catch (const std::exception &e) {
        Logger::fatal(e.what());
        return EXIT_FAILURE;
    }
}
