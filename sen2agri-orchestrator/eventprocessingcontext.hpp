#pragma once

#include <vector>
#include <QFileInfo>

#include "model.hpp"
#include "persistencemanager_interface.h"

class EventProcessingContext
{
    OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient;

public:
    EventProcessingContext(OrgEsaSen2agriPersistenceManagerInterface &persistenceManagerClient);

    JobConfigurationParameterValueList GetJobConfigurationParameters(int jobId, QString prefix);

    int SubmitTask(const NewTask &task);
    void SubmitSteps(int taskId, const NewStepList &steps);

    void MarkJobPaused(int jobId);
    void MarkJobResumed(int jobId);
    void MarkJobCancelled(int jobId);
    void MarkJobFinished(int jobId);
    void MarkJobFailed(int jobId);
    void MarkJobNeedsInput(int jobId);

    TaskIdList GetJobTasksByStatus(int jobId, const ExecutionStatusList &statusList);
    JobStepToRunList GetJobStepsForResume(int jobId);

    UnprocessedEventList GetNewEvents();
    void MarkEventProcessingStarted(int eventId);
    void MarkEventProcessingComplete(int eventId);

    int InsertProduct(const NewProduct &product);

    std::vector<QString> GetProductFiles(const QString &path, const QString &pattern) const;
    QString GetOutputPath(int jobId, int taskId);

    template <typename F>
    NewStepList CreateStepsFromInput(const QString &inputPath,
                                     const QString &outputPath,
                                     const QString &pattern,
                                     F &&f)
    {
        const auto &input = QDir::cleanPath(inputPath) + QDir::separator();

        NewStepList steps;
        for (const auto &file : GetProductFiles(inputPath, pattern)) {

            steps.push_back(
                { QFileInfo(file).baseName(),
                  QString::fromUtf8(QJsonDocument(f(input + file, outputPath + file)).toJson()) });
        }

        return steps;
    }

private:
    QString GetScratchPath(int jobId);
};
