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
    const auto &parametersDoc = QJsonDocument::fromJson(event.parametersJson.toUtf8());
    if (!parametersDoc.isObject()) {
        throw std::runtime_error(
            QStringLiteral("Unexpected step parameter JSON schema: root node should be an "
                           "object. The parameter JSON was: '%1'")
                .arg(event.parametersJson)
                .toStdString());
    }

    const auto &parametersObj = parametersDoc.object();
    const auto &inputPath = parametersObj[QStringLiteral("input_path")].toString();

    if (inputPath.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral("Missing job input path. The parameter JSON was: '%1'")
                .arg(event.parametersJson)
                .toStdString());
    }

    auto taskId =
        ctx.SubmitTask({ event.jobId, QStringLiteral("dummy-module"), QStringLiteral("null") });

    const auto &outputPath = ctx.GetOutputPath(event.jobId, taskId);

    const auto &steps = ctx.CreateStepsFromInput(
        inputPath, outputPath, QStringLiteral("*.*"),
        [](const QString &inputFile, const QString &outputFile) {
            QJsonObject node;
            node[QStringLiteral("arguments")] = QJsonArray{ inputFile, outputFile };

            return node;
        });

    ctx.SubmitSteps(taskId, steps);
}

void DummyProcessorHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                                   const TaskFinishedEvent &event)
{
    ctx.InsertProduct({ ProductType::TestProduct,
                        event.processorId,
                        event.taskId,
                        ctx.GetOutputPath(event.jobId, event.taskId),
                        QDateTime::currentDateTimeUtc() });
}
