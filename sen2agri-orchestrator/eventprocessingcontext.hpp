#pragma once

#include <map>
#include <vector>
#include <QFileInfo>

#include "persistencemanager.hpp"
#include "model.hpp"

#include "tasktosubmit.hpp"

class EventProcessingContext
{
    PersistenceManagerDBProvider &persistenceManager;

public:
    EventProcessingContext(PersistenceManagerDBProvider &persistenceManager);

    std::map<QString, QString> GetJobConfigurationParameters(int jobId, QString prefix);

    int SubmitTask(const NewTask &task);
    void SubmitSteps(const NewStepList &steps);

    void MarkJobPaused(int jobId);
    void MarkJobResumed(int jobId);
    void MarkJobCancelled(int jobId);
    void MarkJobFinished(int jobId);
    void MarkJobFailed(int jobId);
    void MarkJobNeedsInput(int jobId);

    TaskIdList GetJobTasksByStatus(int jobId, const ExecutionStatusList &statusList);
    JobStepToRunList GetTaskStepsForStart(int taskId);
    JobStepToRunList GetJobStepsForResume(int jobId);

    StepConsoleOutputList GetTaskConsoleOutputs(int taskId);

    UnprocessedEventList GetNewEvents();
    void MarkEventProcessingStarted(int eventId);
    void MarkEventProcessingComplete(int eventId);

    int InsertProduct(const NewProduct &product);

    QStringList GetProductFiles(const QString &path, const QString &pattern) const;
    QString GetOutputPath(int jobId, int taskId, const QString &module);

    void SubmitTasks(int jobId, const QList<std::reference_wrapper<TaskToSubmit>> &tasks);

    template <typename F>
    NewStepList CreateStepsFromInput(int taskId,
                                     const QString &inputPath,
                                     const QString &outputPath,
                                     const QString &pattern,
                                     F &&f)
    {
        const auto &input = QDir::cleanPath(inputPath) + QDir::separator();

        NewStepList steps;
        for (const auto &file : GetProductFiles(inputPath, pattern)) {
            steps.push_back(
                { taskId, QFileInfo(file).baseName(),
                  QString::fromUtf8(QJsonDocument(f(input + file, outputPath + file)).toJson()) });
        }

        return steps;
    }

    static QString findProductFile(const QString &path);

private:
    QString GetScratchPath(int jobId);
};
