#include <utility>

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

#include "tasktosubmit.hpp"

static QString getStepJson(const QStringList &arguments);

TaskToSubmit::TaskToSubmit(QString moduleName,
                           QList<std::reference_wrapper<const TaskToSubmit>> parentTasks)
    : taskId(), moduleName(std::move(moduleName)), parentTasks(std::move(parentTasks))
{
}

QString TaskToSubmit::GetFilePath(const QString &file)
{
    return outputPath + file;
}

NewStep TaskToSubmit::CreateStep(QString name, const QStringList &arguments)
{
    return { taskId, std::move(name), getStepJson(arguments) };
}

static QString getStepJson(const QStringList &arguments)
{
    QJsonObject node;
    node[QStringLiteral("arguments")] = QJsonArray::fromStringList(arguments);
    return QString::fromUtf8(QJsonDocument(std::move(node)).toJson());
}
