#include <QDBusPendingCallWatcher>

#include <optional.hpp>

#include "orchestrator.hpp"
#include "settings.hpp"
#include "configuration.hpp"

#include "make_unique.hpp"
#include "processor/cropmaskhandler.hpp"
#include "processor/croptypehandler.hpp"
#include "processor/compositehandler.hpp"
#include "processor/lairetrievalhandler.hpp"
#include "processor/phenondvihandler.hpp"
#include "processor/dummyprocessorhandler.hpp"
#include "json_conversions.hpp"

std::map<int, std::unique_ptr<ProcessorHandler>> & GetHandlersMap(PersistenceManagerDBProvider &persistenceManager) {
    ProcessorDescriptionList processorsDescriptions = persistenceManager.GetProcessorDescriptions();
    static std::map<int, std::unique_ptr<ProcessorHandler>> handlersMap;
    for(ProcessorDescription procDescr: processorsDescriptions) {
        if(procDescr.shortName == "l2a") {
            // TODO:
            //handlers.emplace(procDescr.processorId, std::make_unique<MACCSHandler>());
        } else if(procDescr.shortName == "l3a") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<CompositeHandler>());
        } else if(procDescr.shortName == "l3b") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<LaiRetrievalHandler>());
        } else if(procDescr.shortName == "l3b_pheno") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<PhenoNdviHandler>());
        } else if(procDescr.shortName == "l4a") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<CropMaskHandler>());
        } else if(procDescr.shortName == "l4b") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<CropTypeHandler>());
        } else if(procDescr.shortName == "dummy") {
            handlersMap.emplace(procDescr.processorId, std::make_unique<DummyProcessorHandler>());
        } else {
            throw std::runtime_error(
                QStringLiteral("Invalid processor configuration found in database: %1, exiting.")
                    .arg(procDescr.shortName).toStdString());
        }
    }

    return handlersMap;
}


Orchestrator::Orchestrator(QObject *parent)
    : QObject(parent),
      persistenceManager(
          Settings::readSettings(getConfigurationFile(*QCoreApplication::instance()))),
      executorClient(OrgEsaSen2agriProcessorsExecutorInterface::staticInterfaceName(),
                     QStringLiteral("/org/esa/sen2agri/processorsExecutor"),
                     QDBusConnection::systemBus()),
      worker(GetHandlersMap(persistenceManager), persistenceManager, executorClient)
{
    worker.RescanEvents();
}

void Orchestrator::NotifyEventsAvailable() { RescanEvents(); }

void Orchestrator::RescanEvents() { worker.RescanEvents(); }

JobDefinition Orchestrator::GetJobDefinition(const ProcessingRequest &request)
{
    const auto &doc = QJsonDocument::fromJson(request.parametersJson.toUtf8());
    if (!doc.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: root node should be an "
                           "object. The parameter JSON was: '%1'").arg(request.parametersJson).toStdString());
    }
    const auto &inObj = doc.object();
    // Get the general parameters node
    const auto &generalParamsNode = inObj[QStringLiteral("general_params")];
    if (!generalParamsNode.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: node 'parameters' should be an "
                           "object. The parameter JSON was: '%1'").arg(request.parametersJson).toStdString());
    }
    // Get the start and end date from the parameters
    const auto &generalParamsObj = generalParamsNode.toObject();
    QJsonValue startDateValue = generalParamsObj.value(QStringLiteral("start_date"));
    QJsonValue endDateValue = generalParamsObj.value(QStringLiteral("end_date"));
    QString startDate = startDateValue.isDouble() ? QString::number(startDateValue.toInt()) : startDateValue.toString();
    QString endDate = endDateValue.isDouble() ? QString::number(endDateValue.toInt()) : endDateValue.toString();
    QDateTime startDateTime = QDateTime::fromString(startDate, "yyyyMMdd");
    QDateTime endDateTime = QDateTime::fromString(endDate, "yyyyMMdd");
    // Get all available products for the given site (for all processors) and let each handler decide what it needs
    // TODO: this is not quite OK as we usually need only the products from the previous level
    // but we might have situations when we need a product from CropMask needed by a CropType
    ProductList availableProducts = persistenceManager.GetProducts(request.siteId, -1, startDateTime, endDateTime);

    // Get the specific processor information node
    const auto &processorParamsNode = inObj[QStringLiteral("processor_params")];
    if (!processorParamsNode.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: node 'processor_params' should be an "
                           "object. The parameter JSON was: '%1'").arg(request.parametersJson).toStdString());
    }
    const QJsonObject &processorParamsObj = processorParamsNode.toObject();
    ProcessorHandler &handler = worker.GetHandler(request.processorId);

    // Get the job definition from the processor handler
    JobDefinition jobDef;
    jobDef.processorId = request.processorId;
    jobDef.siteId = request.siteId;
    QString processingDefJson = handler.GetProcessingDefinitionJson(processorParamsObj, availableProducts, jobDef.isValid);
    if(jobDef.isValid) {
        // Create a new object
        QJsonObject retObj(inObj);
        // overwrite the processor params with the one from the processor
        retObj[QStringLiteral("processor_params")] = stringToJsonObject(processingDefJson);
        jobDef.jobDefinitionJson = jsonToString(retObj);
    } else {
        // the returned value normally should contain the error text
        jobDef.jobDefinitionJson = processingDefJson;
    }
    return jobDef;
}

void Orchestrator::SubmitJob(const JobDefinition &job)
{
    const auto &doc = QJsonDocument::fromJson(job.jobDefinitionJson.toUtf8());
    if (!doc.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: root node should be an "
                           "object. The parameter JSON was: '%1'").arg(job.jobDefinitionJson).toStdString());
    }
    const auto &object = doc.object();
    // Get the general parameters node
    const auto &generalParamsNode = object[QStringLiteral("general_params")];
    if (!generalParamsNode.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: node 'general_params' should be an "
                           "object. The parameter JSON was: '%1'").arg(job.jobDefinitionJson).toStdString());
    }
    const auto &generalParamsObj = generalParamsNode.toObject();
    QString taskName = generalParamsObj["name"].toString();
    QString taskDescr = generalParamsObj["description"].toString();
    QString taskStartType = generalParamsObj["type"].toString();

    NewJob newJob;
    newJob.processorId = job.processorId;
    newJob.name = taskName;
    newJob.description = taskDescr;

    newJob.siteId = job.siteId;
    if(taskStartType == "requested")
        newJob.startType = JobStartType::Requested;
    else if(taskStartType == "triggered")
        newJob.startType = JobStartType::Triggered;
    else
        newJob.startType = JobStartType::Scheduled;

    const auto &processorParamsNode = object[QStringLiteral("processor_params")];
    if (!processorParamsNode.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: node 'processor_params' should be an "
                           "object. The parameter JSON was: '%1'").arg(job.jobDefinitionJson).toStdString());
    }
    QJsonObject processorParamsObj(processorParamsNode.toObject());
    // Add also the site_name and processor_name to the parameteres
    QString processorName = persistenceManager.GetProcessorShortName(job.processorId);
    if(processorName.length() == 0)
        processorName = "UnknownProcessor";
    QString siteName = persistenceManager.GetSiteName(job.siteId);
    if(siteName.length() == 0)
        siteName = "UnknownSite";
    processorParamsObj["processor_short_name"] = processorName;
    processorParamsObj["site_name"] = siteName;
    newJob.parametersJson = jsonToString(processorParamsObj);

    const auto &configParamNode = object[QStringLiteral("config_param")];
    if (!configParamNode.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: node 'processor_config' should be an "
                           "object. The parameter JSON was: '%1'").arg(job.jobDefinitionJson).toStdString());
    }
    const auto &configParamObj = configParamNode.toObject();

    for(QJsonObject::const_iterator iter = configParamObj.begin(); iter != configParamObj.end (); ++iter)
    {
        newJob.configuration.append(JobConfigurationUpdateAction(iter.key(), iter.value().toString()));
    }

    persistenceManager.SubmitJob(newJob);
    NotifyEventsAvailable();
}

