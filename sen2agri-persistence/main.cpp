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
#include "configuration.hpp"
#include "settings.hpp"
#include "logger.hpp"

template <typename T>
void printSignature()
{
    qDebug() << QMetaType::typeName(qMetaTypeId<T>())
             << QDBusMetaType::typeToSignature(qMetaTypeId<T>());
}

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(QStringLiteral("sen2agri-persistence"));

        const auto &configFile = getConfigurationFile(app);

        Logger::info(QStringLiteral("Reading settings from %1").arg(configFile));

        if (!QFileInfo::exists(configFile)) {
            throw std::runtime_error(
                QStringLiteral("Configuration file %1 does not exist, exiting.")
                    .arg(configFile)
                    .toStdString());
        }

        const auto &settings = Settings::readSettings(configFile);

        registerMetaTypes();

        printSignature<NewProduct>();

        auto connection = QDBusConnection::systemBus();
        PersistenceManager persistenceManager(settings);

        new PersistenceManagerAdaptor(&persistenceManager);

        if (!connection.registerObject(QStringLiteral("/org/esa/sen2agri/persistenceManager"),
                                       &persistenceManager)) {
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
