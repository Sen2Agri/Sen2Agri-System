#include <stdexcept>

#include <QCoreApplication>
#include <QDBusConnection>

#include <model.hpp>
#include <logger.hpp>

#include "orchestrator.hpp"
#include "orchestrator_adaptor.h"

int main(int argc, char *argv[])
{
    try {
        QCoreApplication app(argc, argv);

        registerMetaTypes();

        auto connection = QDBusConnection::systemBus();
        Orchestrator orchestrator;

        new OrchestratorAdaptor(&orchestrator);

        if (!connection.registerObject(QStringLiteral("/"), &orchestrator)) {
            throw std::runtime_error(
                QStringLiteral("Error registering the object with D-Bus: %1, exiting.")
                    .arg(connection.lastError().message())
                    .toStdString());
        }

        if (!connection.registerService(QStringLiteral("org.esa.sen2agri.orchestrator"))) {
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
