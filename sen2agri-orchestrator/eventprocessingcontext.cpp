#include <QDir>
#include <QFile>
#include <QString>

#include "dbus_future_utils.hpp"

#include "eventprocessingcontext.hpp"

EventProcessingContext::EventProcessingContext(PersistenceManagerDBProvider &persistenceManager)
    : persistenceManager(persistenceManager)
{
}

std::map<QString, QString> EventProcessingContext::GetJobConfigurationParameters(int jobId,
                                                                                 QString prefix)
{
    const auto &paramList = persistenceManager.GetJobConfigurationParameters(jobId, prefix);

    std::map<QString, QString> result;
    for (const auto &p : paramList) {
        result.emplace(p.key, p.value);
    }

    return result;
}

int EventProcessingContext::SubmitTask(const NewTask &task, const QString &submitterProcName)
{
    auto taskId = persistenceManager.SubmitTask(task);

    const auto &path = GetOutputPath(task.jobId, taskId, task.module, submitterProcName);
    if (!QDir::root().mkpath(path)) {
        throw std::runtime_error(
            QStringLiteral("Unable to create job output path %1").arg(path).toStdString());
    }

    return taskId;
}

void EventProcessingContext::SubmitSteps(const NewStepList &steps)
{
    persistenceManager.SubmitSteps(steps);
}

void EventProcessingContext::MarkJobPaused(int jobId) { persistenceManager.MarkJobPaused(jobId); }

void EventProcessingContext::MarkJobResumed(int jobId) { persistenceManager.MarkJobResumed(jobId); }

void EventProcessingContext::MarkJobCancelled(int jobId)
{
    persistenceManager.MarkJobCancelled(jobId);
}

void EventProcessingContext::MarkJobFinished(int jobId)
{
    persistenceManager.MarkJobFinished(jobId);
}

void EventProcessingContext::MarkJobFailed(int jobId) { persistenceManager.MarkJobFailed(jobId); }

void EventProcessingContext::MarkJobNeedsInput(int jobId)
{
    persistenceManager.MarkJobNeedsInput(jobId);
}

TaskIdList EventProcessingContext::GetJobTasksByStatus(int jobId,
                                                       const ExecutionStatusList &statusList)
{
    return persistenceManager.GetJobTasksByStatus(jobId, statusList);
}

JobStepToRunList EventProcessingContext::GetTaskStepsForStart(int taskId)
{
    return persistenceManager.GetTaskStepsForStart(taskId);
}

JobStepToRunList EventProcessingContext::GetJobStepsForResume(int jobId)
{
    return persistenceManager.GetJobStepsForResume(jobId);
}

StepConsoleOutputList EventProcessingContext::GetTaskConsoleOutputs(int taskId)
{
    return persistenceManager.GetTaskConsoleOutputs(taskId);
}

UnprocessedEventList EventProcessingContext::GetNewEvents()
{
    return persistenceManager.GetNewEvents();
}

void EventProcessingContext::MarkEventProcessingStarted(int eventId)
{
    persistenceManager.MarkEventProcessingStarted(eventId);
}

void EventProcessingContext::MarkEventProcessingComplete(int eventId)
{
    persistenceManager.MarkEventProcessingComplete(eventId);
}

int EventProcessingContext::InsertProduct(const NewProduct &product)
{
    return persistenceManager.InsertProduct(product);
}

QStringList EventProcessingContext::GetProductFiles(const QString &path,
                                                    const QString &pattern) const
{
    return QDir(path).entryList(QStringList() << pattern, QDir::Files);
}

QString EventProcessingContext::GetJobOutputPath(int jobId, const QString &procName)
{
    QString jobIdPath = GetScratchPath(jobId, procName);
    jobIdPath = jobIdPath.left(jobIdPath.indexOf("{job_id}") + sizeof("{job_id}"));
    return jobIdPath.replace(QLatin1String("{job_id}"), QString::number(jobId));
}

QString EventProcessingContext::GetOutputPath(int jobId, int taskId, const QString &module, const QString &procName)
{
    return GetScratchPath(jobId, procName)
        .replace(QLatin1String("{job_id}"), QString::number(jobId))
        .replace(QLatin1String("{task_id}"), QString::number(taskId))
        .replace(QLatin1String("{module}"), module);
}

QString EventProcessingContext::GetScratchPath(int jobId, const QString &procName)
{
    const auto &parameters = GetJobConfigurationParameters(
        jobId, QStringLiteral("general.scratch-path"));

    if (parameters.empty()) {
        throw std::runtime_error("Please configure the \"general.scratch-path\" parameter with the "
                                 "temporary file path");
    }

    Q_ASSERT(parameters.size() >= 1);
    std::map<QString,QString>::const_iterator it = parameters.find(QStringLiteral("general.scratch-path"));
    if (procName != "") {
        QString newKey = "general.scratch-path."+procName;
        std::map<QString,QString>::const_iterator it1 = parameters.find(newKey);
        if(it1 != parameters.end()) {
            if(it1->second != "") {
                it = it1;
            }
        }
    }
    Q_ASSERT(it != parameters.end());

    return QDir::cleanPath(it->second) + QDir::separator();
}

void EventProcessingContext::SubmitTasks(int jobId,
                                         const QList<std::reference_wrapper<TaskToSubmit>> &tasks,
                                         const QString &submitterProcName)
{
    for (auto taskRef : tasks) {
        auto &task = taskRef.get();

        QList<int> parentTaskIds;
        parentTaskIds.reserve(task.parentTasks.size());
        for (auto parentTaskRef : task.parentTasks) {
            auto &parentTask = parentTaskRef.get();

            if (!parentTask.taskId) {
                throw std::runtime_error(
                    "Please sort the tasks according to their execution order");
            }

            parentTaskIds.append(parentTask.taskId);
        }

        task.taskId = SubmitTask({ jobId, task.moduleName, QStringLiteral("null"), parentTaskIds },
                                 submitterProcName);

        task.outputPath = GetOutputPath(jobId, task.taskId, task.moduleName, submitterProcName);
    }
}

ProductList EventProcessingContext::GetProducts(int siteId, int productTypeId, const QDateTime &startDate, const QDateTime &endDate)
{
    return persistenceManager.GetProducts(siteId, productTypeId, startDate, endDate);
}

ProductList EventProcessingContext::GetProductsForTile(const QString &tileId, ProductType productType,
                                                       int tileSatelliteId, int targetSatelliteId)
{
    return persistenceManager.GetProductsForTile(tileId, productType, tileSatelliteId, targetSatelliteId);
}

TileList EventProcessingContext::GetIntersectingTiles(Satellite satellite, const QString &tileId)
{
    return persistenceManager.GetIntersectingTiles(satellite, tileId);
}

QString EventProcessingContext::GetProductAbsolutePath(const QString &path) {
    QFileInfo fileInfo(path);
    QString absPath = path;
    if(!fileInfo.isAbsolute()) {
        // if we have the product name, we need to get the product path from the database
        Product product = persistenceManager.GetProduct(path);
        absPath = product.fullPath;
    }
    return absPath;
}

QStringList EventProcessingContext::findProductFiles(const QString &path)
{
    QStringList result;
    QFileInfo fileInfo(path);
    QString absPath = path;
    if(!fileInfo.isAbsolute()) {
        // if we have the product name, we need to get the product path from the database
        Product product = persistenceManager.GetProduct(path);
        absPath = product.fullPath;
    }
    for (const auto &file : QDir(absPath).entryList({ "*.HDR", "*.xml" }, QDir::Files)) {
        result.append(absPath + file);
    }

    if (result.isEmpty()) {
        throw std::runtime_error(
            QStringLiteral(
                "Unable to find an HDR or xml file in path %1. Unable to determine the product "
                "metadata file.")
                .arg(absPath)
                .toStdString());
    }
    return result;
}

QString EventProcessingContext::GetProcessorShortName(int processorId)
{
    return persistenceManager.GetProcessorShortName(processorId);
}

QString EventProcessingContext::GetSiteShortName(int siteId)
{
    return persistenceManager.GetSiteShortName(siteId);
}

QString EventProcessingContext::GetSiteName(int siteId)
{
    return persistenceManager.GetSiteName(siteId);
}
