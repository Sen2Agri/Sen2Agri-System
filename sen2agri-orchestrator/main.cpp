#include <stdexcept>

#include <QCoreApplication>
#include <QDBusConnection>

#include "make_unique.hpp"
#include "model.hpp"
#include "logger.hpp"

#include "orchestrator.hpp"
#include "orchestrator_adaptor.h"

#include "processor/cropmaskhandler.hpp"
#include "processor/croptypehandler.hpp"
#include "processor/compositehandler.hpp"
#include "processor/lairetrievalhandler.hpp"
#include "processor/phenondvihandler.hpp"
#include "processor/dummyprocessorhandler.hpp"

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QCoreApplication app(argc, argv);

        registerMetaTypes();

        std::map<int, std::unique_ptr<ProcessorHandler>> handlers;
        handlers.emplace(COMPOSITE_ID, std::make_unique<CompositeHandler>());
        handlers.emplace(LAI_RETRIEVAL_ID, std::make_unique<LaiRetrievalHandler>());
        handlers.emplace(PHENO_NDVI_ID, std::make_unique<PhenoNdviHandler>());
        handlers.emplace(CROP_MASK_ID, std::make_unique<CropMaskHandler>());
        handlers.emplace(CROP_TYPE_ID, std::make_unique<CropTypeHandler>());
        handlers.emplace(DUMMY_HANDLER_ID, std::make_unique<DummyProcessorHandler>());

        Orchestrator orchestrator(handlers);

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
