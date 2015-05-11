#include "configmodel.h"

#include <QJsonObject>
#include <QVariantMap>

QJsonDocument ConfigModel::serialize() const
{
    QJsonObject root;
    QJsonObject process1Node;
    process1Node["value1"] = value1;
    process1Node["value2"] = value2;
    root["Process1"] = process1Node;
    QJsonObject process2Node;
    root["Process2"] = process2Node;
    QJsonObject process3Node;
    root["Process3"] = process3Node;
    QJsonObject process4Node;
    root["Process4"] = process4Node;
    return QJsonDocument(root);
}

ConfigModel ConfigModel::deserialize(const QJsonDocument &document)
{
    const QJsonObject &root = document.object();
    const QJsonObject &process1Node = root["Process1"].toObject();
    const QJsonObject &process2Node = root["Process2"].toObject();
    const QJsonObject &process3Node = root["Process3"].toObject();
    const QJsonObject &process4Node = root["Process4"].toObject();

    return ConfigModel{ process1Node["value1"].toString(),
                        process1Node["value2"].toString() };
}
