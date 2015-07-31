#pragma once

#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

QString jsonToString(const QJsonDocument &doc);
QString jsonToString(const QJsonObject &obj);
QString jsonToString(const QJsonArray &arr);

QJsonDocument stringToJsonDocument(const QString &s);
QJsonObject stringToJsonObject(const QString &s);
QJsonArray stringToJsonArray(const QString &s);
