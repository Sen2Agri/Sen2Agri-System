#pragma once

#include <QCoreApplication>
#include <QString>

QString getConfigurationFile(const QCoreApplication &app);
QString getConfigurationFileEx(const QCoreApplication &app, const QString &preferredFileName);
