#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>

#include "settings.hpp"

class Monitor : public QObject
{
    Q_OBJECT

    QNetworkAccessManager networkAccessManager;
    QTimer timer;
    QString serviceUrl;
    QString diskPath;
    bool isConfigured;

public:
    explicit Monitor(const Settings &settings, QObject *parent = 0);

private:
    void getConfiguration();
    void sendStatistics();

    static QString defaultDiskPath;
    static constexpr int defaultScanInterval = 60000;

private slots:
    void configurationRead();
    void timerFired();
    void sendFinished();
};
