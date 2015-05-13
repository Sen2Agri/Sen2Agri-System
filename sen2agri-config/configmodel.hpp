#pragma once

#include <QString>
#include <QJsonDocument>

class ConfigModel
{
public:
    QString value1;
    QString value2;

    QJsonDocument serialize() const;

    static ConfigModel deserialize(const QJsonDocument &document);
};
