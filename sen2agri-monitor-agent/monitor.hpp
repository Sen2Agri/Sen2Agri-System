#pragma once

#include <QObject>
#include <QString>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QSettings>

class Monitor : public QObject
{
    Q_OBJECT

    QNetworkAccessManager networkAccessManager;
    QTimer timer;
    QString serviceUrl;
    QString diskPath;
    int scanInterval;
    bool isConfigured;

public:
    explicit Monitor(const QSettings &settings, QObject *parent = 0);

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
