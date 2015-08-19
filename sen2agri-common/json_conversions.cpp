#include "json_conversions.hpp"

QString jsonToString(const QJsonDocument &doc)
{
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

QString jsonToString(const QJsonObject &obj) { return jsonToString(QJsonDocument(obj)); }

QString jsonToString(const QJsonArray &arr) { return jsonToString(QJsonDocument(arr)); }

QJsonDocument stringToJsonDocument(const QString &s) { return QJsonDocument::fromJson(s.toUtf8()); }

QJsonObject stringToJsonObject(const QString &s) { return stringToJsonDocument(s).object(); }

QJsonArray stringToJsonArray(const QString &s) { return stringToJsonDocument(s).array(); }
