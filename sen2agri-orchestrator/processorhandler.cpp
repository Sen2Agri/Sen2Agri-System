#include "processorhandler.hpp"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QFileInfoList>

bool removeDir(const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

ProcessorHandler::~ProcessorHandler() {}

void ProcessorHandler::HandleProductAvailable(EventProcessingContext &ctx,
                                              const ProductAvailableEvent &event)
{
    HandleProductAvailableImpl(ctx, event);
}

void ProcessorHandler::HandleJobSubmitted(EventProcessingContext &ctx,
                                          const JobSubmittedEvent &event)
{
    HandleJobSubmittedImpl(ctx, event);
}

void ProcessorHandler::HandleTaskFinished(EventProcessingContext &ctx,
                                          const TaskFinishedEvent &event)
{
    HandleTaskFinishedImpl(ctx, event);
}

void ProcessorHandler::HandleProductAvailableImpl(EventProcessingContext &,
                                                  const ProductAvailableEvent &)
{
}

QString ProcessorHandler::GetProcessingDefinitionJson(const QJsonObject &procInfoParams, const ProductList &listProducts, bool &bIsValid) {
    return GetProcessingDefinitionJsonImpl(procInfoParams, listProducts, bIsValid);
}

QString ProcessorHandler::GetFinalProductFolder(EventProcessingContext &ctx, int jobId,
                                                int processorId, int siteId) {
    auto configParameters = ctx.GetJobConfigurationParameters(
                jobId, "archiver.archive_path");

    QString processorName = ctx.GetProcessorShortName(processorId);
    QString siteName = ctx.GetSiteName(siteId);
    auto it = configParameters.find("archiver.archive_path");
    if (it == std::end(configParameters)) {
        throw std::runtime_error(QStringLiteral("No final product folder configured for site %1 and processor %2")
                                     .arg(siteName)
                                     .arg(processorName)
                                     .toStdString());
    }
    QString folderName = (*it).second;
    folderName = folderName.replace("{site}", siteName);
    folderName = folderName.replace("{processor}", processorName);

    return folderName;
}

bool ProcessorHandler::RemoveJobFolder(EventProcessingContext &ctx, int jobId)
{
    QString jobOutputPath = ctx.GetOutputPath(jobId);
    return removeDir(jobOutputPath);
}
