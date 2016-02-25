#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "dummyprocessorhandler.hpp"

void DummyProcessorHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
                                                       const ProductAvailableEvent &event)
{
    Q_UNUSED(ctx);
    Q_UNUSED(event);
}

void DummyProcessorHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                                   const JobSubmittedEvent &event)
{
    const auto &parameters = QJsonDocument::fromJson(event.parametersJson.toUtf8()).object();

    auto inputPath = parameters["input_path"].toString();
    if (inputPath.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral("Missing job input path. The parameter JSON was: '%1'")
                .arg(event.parametersJson)
                .toStdString());
    }

    inputPath = QDir::cleanPath(inputPath) + QDir::separator();

    TaskToSubmit task1{ QStringLiteral("dummy-module"), {} };
    TaskToSubmit task2{ QStringLiteral("dummy-module"), {} };
    TaskToSubmit task3{ QStringLiteral("dummy-module"), { task1, task2 } };

    ctx.SubmitTasks(event.jobId, { task1, task2, task3 });

    NewStepList steps;
    for (const auto &file : ctx.GetProductFiles(inputPath, "*.*")) {
        const auto &inputFile = inputPath + file;
        const auto &stepName = QFileInfo(file).baseName();

        steps.push_back(task1.CreateStep(stepName, { inputFile, task1.GetFilePath(file) }));
        steps.push_back(task2.CreateStep(stepName, { inputFile, task2.GetFilePath(file) }));
        steps.push_back(task3.CreateStep(stepName, { inputFile, task3.GetFilePath(file) }));
    }

    ctx.SubmitSteps(steps);
}

void DummyProcessorHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                                   const TaskFinishedEvent &event)
{
    ctx.InsertProduct({ ProductType::TestProductTypeId, event.processorId, event.taskId,
                        ctx.GetOutputPath(event.jobId, event.taskId, event.module),
                        QDateTime::currentDateTimeUtc() });
}

ProcessorJobDefinitionParams DummyProcessorHandler::GetProcessingDefinitionImpl(SchedulingContext &ctx,
                                                                                    int siteId, int productTime,
                                                const ConfigurationParameterValueMap &requestOverrideCfgValues)
{
    Q_UNUSED(ctx);
    Q_UNUSED(siteId);

    //bIsValid = false;

    //return QString("Cannot execute DummyProcessor processor.!");
}
