#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler_new.hpp"
#include "processorhandlerhelper.h"
#include "json_conversions.hpp"
#include "logger.hpp"

// The number of tasks that are executed for each product before executing time series tasks
#define LAI_TASKS_PER_PRODUCT       6
#define MODEL_GEN_TASKS_PER_PRODUCT 4
#define CUT_TASKS_NO                5

#define DEFAULT_GENERATED_SAMPLES_NO    "40000"
#define DEFAULT_NOISE_VAR               "0.01"
#define DEFAULT_BEST_OF                 "1"
#define DEFAULT_REGRESSOR               "nn"

void LaiRetrievalHandlerNew::SetProcessorDescription(const ProcessorDescription &procDescr) {
    this->processorDescr = procDescr;
    m_l3bHandler.SetProcessorDescription(procDescr);
    m_l3cHandler.SetProcessorDescription(procDescr);
}

void LaiRetrievalHandlerNew::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();
    std::map<QString, QString> configParameters = ctx.GetJobConfigurationParameters(event.jobId, "processor.l3b.");

    bool bMonoDateLai = IsGenMonoDate(parameters, configParameters);
    bool bNDayReproc = IsNDayReproc(parameters, configParameters);
    bool bFittedReproc = IsFittedReproc(parameters, configParameters);
    if(!bNDayReproc && !bFittedReproc && !bMonoDateLai) {
        ctx.MarkJobFailed(event.jobId);
        throw std::runtime_error(
            QStringLiteral("At least one processing needs to be defined (LAI mono-date,"
                           " LAI N-reprocessing or LAI Fitted)").toStdString());
    }
    if(bMonoDateLai) {
        m_l3bHandler.HandleJobSubmitted(ctx, event);
    } else if(bNDayReproc || bFittedReproc) {
        m_l3cHandler.HandleJobSubmitted(ctx, event);
    }
}

void LaiRetrievalHandlerNew::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    this->m_l3bHandler.HandleTaskFinished(ctx, event);
    this->m_l3cHandler.HandleTaskFinished(ctx, event);
}

ProcessorJobDefinitionParams LaiRetrievalHandlerNew::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                          const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;

    QDateTime seasonStartDate;
    QDateTime seasonEndDate;
    // extract the scheduled date
    QDateTime qScheduledDate = QDateTime::fromTime_t(scheduledDate);
    GetSeasonStartEndDates(ctx, siteId, seasonStartDate, seasonEndDate, qScheduledDate, requestOverrideCfgValues);
    QDateTime limitDate = seasonEndDate.addMonths(2);
    if(qScheduledDate > limitDate) {
        return params;
    }
    if(!seasonStartDate.isValid()) {
        Logger::error(QStringLiteral("Season start date for site ID %1 is invalid in the database!")
                      .arg(siteId));
        return params;
    }

    ConfigurationParameterValueMap mapCfg = ctx.GetConfigurationParameters(QString("processor.l3b."), siteId, requestOverrideCfgValues);

    // we might have an offset in days from starting the downloading products to start the L3B/L3C/L3D production
    int startSeasonOffset = mapCfg["processor.l3b.start_season_offset"].value.toInt();
    seasonStartDate = seasonStartDate.addDays(startSeasonOffset);

    int generateLai = false;
    int generateReprocess = false;
    int generateFitted = false;

    if(requestOverrideCfgValues.contains("product_type")) {
        const ConfigurationParameterValue &productType = requestOverrideCfgValues["product_type"];
        if(productType.value == "L3B") {
            generateLai = true;
            params.jsonParameters = "{ \"monolai\": \"1\"}";
        } else if(productType.value == "L3C") {
            generateReprocess = true;
            params.jsonParameters = "{ \"reproc\": \"1\"}";
        } else if(productType.value == "L3D") {
            generateFitted = true;
            params.jsonParameters = "{ \"fitted\": \"1\"}";
        }
    }
    // we need to have at least one flag set
    if(!generateLai && !generateReprocess && !generateFitted) {
        return params;
    }

    // by default the start date is the season start date
    QDateTime startDate = seasonStartDate;
    QDateTime endDate = qScheduledDate;

    if(generateLai || generateReprocess) {
        int productionInterval = mapCfg[generateLai ? "processor.l3b.production_interval":
                                                      "processor.l3b.reproc_production_interval"].value.toInt();
        startDate = endDate.addDays(-productionInterval);
        // Use only the products after the configured start season date
        if(startDate < seasonStartDate) {
            startDate = seasonStartDate;
        }
    }

    params.productList = ctx.GetProducts(siteId, (int)ProductType::L2AProductTypeId, startDate, endDate);
    // Normally, we need at least 1 product available in order to be able to create a L3B/L3C/L3D product
    // but if we do not return here, the schedule block waiting for products (that might never happen)
    bool waitForAvailProcInputs = (mapCfg["processor.l3b.sched_wait_proc_inputs"].value.toInt() != 0);
    if((waitForAvailProcInputs == false) || (params.productList.size() > 0)) {
        params.isValid = true;
//        if(generateLai) {
//            params.isValid = true;
//        } else if(generateFitted) {
//            if(params.productList.size() > 4) {
//                params.isValid = true;
//            }
//        } else if (generateReprocess > 2) {
//            params.isValid = true;
//        }
        Logger::debug(QStringLiteral("Executing scheduled job. Scheduler extracted for L3B/L3C/L3D a number "
                                     "of %1 products for site ID %2 with start date %3 and end date %4!")
                      .arg(params.productList.size())
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    } else {
        Logger::debug(QStringLiteral("Scheduled job for L3B/L3C/L3D and site ID %1 with start date %2 and end date %3 "
                                     "will not be executed (no products)!")
                      .arg(siteId)
                      .arg(startDate.toString())
                      .arg(endDate.toString()));
    }

    return params;
}

bool LaiRetrievalHandlerNew::IsGenModels(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bGenModels = false;
    if(parameters.contains("genmodel")) {
        bGenModels = (parameters["genmodel"].toInt() != 0);
    } else {
        bGenModels = ((configParameters["processor.l3b.generate_models"]).toInt() != 0);
    }
    return bGenModels;
}

bool LaiRetrievalHandlerNew::IsGenMonoDate(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bMonoDateLai = true;
    if(parameters.contains("monolai")) {
        const auto &value = parameters["monolai"];
        if(value.isDouble())
            bMonoDateLai = (value.toInt() != 0);
        else if(value.isString()) {
            bMonoDateLai = (value.toString() == "1");
        }
    } else {
        bMonoDateLai = ((configParameters["processor.l3b.mono_date_lai"]).toInt() != 0);
    }
    return bMonoDateLai;
}

bool LaiRetrievalHandlerNew::IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bNDayReproc = false;
    if(parameters.contains("reproc")) {
        const auto &value = parameters["reproc"];
        if(value.isDouble())
            bNDayReproc = (value.toInt() != 0);
        else if(value.isString()) {
            bNDayReproc = (value.toString() == "1");
        }
    } else {
        bNDayReproc = ((configParameters["processor.l3b.reprocess"]).toInt() != 0);
    }
    return bNDayReproc;
}

bool LaiRetrievalHandlerNew::IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
    bool bFittedReproc = false;
    if(parameters.contains("fitted")) {
        const auto &value = parameters["fitted"];
        if(value.isDouble())
            bFittedReproc = (value.toInt() != 0);
        else if(value.isString()) {
            bFittedReproc = (value.toString() == "1");
        }
    } else {
        bFittedReproc = ((configParameters["processor.l3b.fitted"]).toInt() != 0);
    }
    return bFittedReproc;
}

