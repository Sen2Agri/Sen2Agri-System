#pragma once

#include <QList>
#include <QString>

#include "model.hpp"

class TaskToSubmit
{
public:
    int taskId;
    QString moduleName;
    QList<std::reference_wrapper<const TaskToSubmit>> parentTasks;
    QString outputPath;

    TaskToSubmit(QString moduleName, QList<std::reference_wrapper<const TaskToSubmit>> parentTasks);

    QString GetFilePath(const QString &file);
    NewStep CreateStep(QString name, const QStringList &arguments);
};
