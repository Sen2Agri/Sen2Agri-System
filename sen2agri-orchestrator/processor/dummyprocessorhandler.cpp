#include <QJsonDocument>
#include <QJsonObject>

#include "dummyprocessorhandler.hpp"

void DummyProcessorHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
                                                       const ProductAvailableEvent &event)
{
    Q_UNUSED(ctx);
    Q_UNUSED(event);
}

static QString getStepJson(const QStringList &arguments)
{
    QJsonObject node;
    node[QStringLiteral("arguments")] = QJsonArray::fromStringList(arguments);
    return QString::fromUtf8(QJsonDocument(std::move(node)).toJson());
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
    auto inputPath = parametersObj[QStringLiteral("input_path")].toString();
    if (inputPath.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral("Missing job input path. The parameter JSON was: '%1'")
                .arg(event.parametersJson)
                .toStdString());
    }

    inputPath = QDir::cleanPath(inputPath) + QDir::separator();

    auto task1Id =
        ctx.SubmitTask({ event.jobId, QStringLiteral("dummy-module"), QStringLiteral("null"), {} });
    auto task2Id =
        ctx.SubmitTask({ event.jobId, QStringLiteral("dummy-module"), QStringLiteral("null"), {} });
    auto task3Id = ctx.SubmitTask({ event.jobId,
                                    QStringLiteral("dummy-module"),
                                    QStringLiteral("null"),
                                    { task1Id, task2Id } });

    const auto &output1Path = ctx.GetOutputPath(event.jobId, task1Id);
    const auto &output2Path = ctx.GetOutputPath(event.jobId, task2Id);
    const auto &output3Path = ctx.GetOutputPath(event.jobId, task3Id);

    NewStepList steps;
    for (const auto &file : ctx.GetProductFiles(inputPath, "*.*")) {
        steps.push_back({ task1Id,
                          QFileInfo(file).baseName(),
                          getStepJson({ inputPath + file, output1Path + file }) });
        steps.push_back({ task2Id,
                          QFileInfo(file).baseName(),
                          getStepJson({ inputPath + file, output2Path + file }) });
        steps.push_back({ task3Id,
                          QFileInfo(file).baseName(),
                          getStepJson({ inputPath + file, output3Path + file }) });
    }

    ctx.SubmitSteps(steps);
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
