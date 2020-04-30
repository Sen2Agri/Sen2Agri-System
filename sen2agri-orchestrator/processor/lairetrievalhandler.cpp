#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <fstream>

#include "lairetrievalhandler.hpp"
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

void LaiRetrievalHandler::SetProcessorDescription(const ProcessorDescription &procDescr) {
    this->processorDescr = procDescr;
    m_ndviHandler.SetProcessorDescription(procDescr);
    m_l3bHandler.SetProcessorDescription(procDescr);
    m_l3bHandlerNew.SetProcessorDescription(procDescr);
    m_l3cHandler.SetProcessorDescription(procDescr);
}

void LaiRetrievalHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
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
        if (IsGenNdviMonoDateOnly(configParameters)) {
            m_ndviHandler.HandleJobSubmittedImpl(ctx, event);
        } else if (IsNewLaiMonoDateVersion(configParameters)) {
            m_l3bHandlerNew.HandleJobSubmittedImpl(ctx, event);
        } else {
            m_l3bHandler.HandleJobSubmittedImpl(ctx, event);
        }
    } else if(bNDayReproc || bFittedReproc) {
        m_l3cHandler.HandleJobSubmittedImpl(ctx, event);
    }
}

void LaiRetrievalHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    this->m_ndviHandler.HandleTaskFinished(ctx, event);
    this->m_l3bHandler.HandleTaskFinished(ctx, event);
    this->m_l3bHandlerNew.HandleTaskFinished(ctx, event);
    this->m_l3cHandler.HandleTaskFinished(ctx, event);
}

ProcessorJobDefinitionParams LaiRetrievalHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx, int siteId, int scheduledDate,
                                                          const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    ProcessorJobDefinitionParams params;
    params.isValid = false;
    std::map<QString, QString> configParameters = ctx.GetConfigurationParameterValues("processor.l3b.");

    if(requestOverrideCfgValues.contains("product_type")) {
        const ConfigurationParameterValue &productType = requestOverrideCfgValues["product_type"];
        if(productType.value == "L3B") {
            if (IsGenNdviMonoDateOnly(configParameters)) {
                return this->m_ndviHandler.GetProcessingDefinition(ctx, siteId, scheduledDate, requestOverrideCfgValues);
            } else if (IsNewLaiMonoDateVersion(configParameters)) {
                return m_l3bHandlerNew.GetProcessingDefinition(ctx, siteId, scheduledDate, requestOverrideCfgValues);
            } else {
                return this->m_l3bHandler.GetProcessingDefinition(ctx, siteId, scheduledDate, requestOverrideCfgValues);
            }
        } else if(productType.value == "L3C" || productType.value == "L3D") {
            return this->m_l3cHandler.GetProcessingDefinition(ctx, siteId, scheduledDate, requestOverrideCfgValues);
        }
    }
    return params;
}

bool LaiRetrievalHandler::IsGenMonoDate(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
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

bool LaiRetrievalHandler::IsGenNdviMonoDateOnly(std::map<QString, QString> &configParameters) {
    return ((configParameters["processor.l3b.mono_date_ndvi_only"]).toInt() != 0);
}

bool LaiRetrievalHandler::IsNewLaiMonoDateVersion(std::map<QString, QString> &configParameters) {
    return ((configParameters["processor.l3b.lai.use_inra_version"]).toInt() != 0);
}

bool LaiRetrievalHandler::IsNDayReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
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

bool LaiRetrievalHandler::IsFittedReproc(const QJsonObject &parameters, std::map<QString, QString> &configParameters) {
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

