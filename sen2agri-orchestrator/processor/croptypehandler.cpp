#include <QJsonDocument>
#include <QJsonObject>

#include "croptypehandler.hpp"

void CropTypeHandler::HandleProductAvailableImpl(EventProcessingContext &ctx,
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

static QString findProductMetadata(const QString &path)
{
    QString result;
    for (const auto &file : QDir(path).entryList({ "*.HDR", "*.xml" }, QDir::Files)) {
        if (!result.isEmpty()) {
            throw std::runtime_error(
                QStringLiteral(
                    "More than one HDR or xml file in path %1. Unable to determine the product "
                    "metadata file.")
                    .arg(path)
                    .toStdString());
        }

        result = file;
    }

    if (result.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral(
                "Unable to find an HDR or xml file in path %1. Unable to determine the product "
                "metadata file.")
                .arg(path)
                .toStdString());
    }

    return path + result;
}

void CropTypeHandler::HandleJobSubmittedImpl(EventProcessingContext &ctx,
                                             const JobSubmittedEvent &event)
{
    const auto &inputProducts =
        QJsonDocument::fromJson(event.parametersJson.toUtf8()).object()["input_products"].toArray();

    auto task1Id = ctx.SubmitTask(
        { event.jobId, QStringLiteral("bands-extractor"), QStringLiteral("null"), {} });

    const auto &output1Path = ctx.GetOutputPath(event.jobId, task1Id);

    NewStepList steps;
    QStringList args = { QStringLiteral("BandsExtractor"),
                         QStringLiteral("-out"),
                         output1Path + QStringLiteral("fts.tif"),
                         QStringLiteral("-mask"),
                         output1Path + QStringLiteral("mask.tif"),
                         QStringLiteral("-outdate"),
                         output1Path + QStringLiteral("dates.txt"),
                         QStringLiteral("-il") };

    for (const auto &inputProduct : inputProducts) {
        args.append(findProductMetadata(inputProduct.toString()));
    }
    qDebug() << args;
    steps.push_back({ task1Id, "BandsExtractor", getStepJson({ args }) });
    ctx.SubmitSteps(steps);
}

void CropTypeHandler::HandleTaskFinishedImpl(EventProcessingContext &ctx,
                                             const TaskFinishedEvent &event)
{
    Q_UNUSED(ctx);
    Q_UNUSED(event);

    //    ctx.InsertProduct({ ProductType::TestProduct,
    //                        event.processorId,
    //                        event.taskId,
    //                        ctx.GetOutputPath(event.jobId, event.taskId),
    //                        QDateTime::currentDateTimeUtc() });
}
