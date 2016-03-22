#include <stdexcept>

#include <QCoreApplication>
#include <QDBusConnection>

#include "make_unique.hpp"
#include "model.hpp"
#include "logger.hpp"

#include "orchestrator.hpp"
#include "orchestrator_adaptor.h"

#define RESCAN_EVENTS_TIMEOUT   10000

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QCoreApplication app(argc, argv);

        registerMetaTypes();

        Orchestrator orchestrator;
        Timer timer(&orchestrator);
        timer.start(RESCAN_EVENTS_TIMEOUT);

        new OrchestratorAdaptor(&orchestrator);

        auto connection = QDBusConnection::systemBus();
        if (!connection.registerObject(QStringLiteral("/org/esa/sen2agri/orchestrator"),
                                       &orchestrator)) {
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
