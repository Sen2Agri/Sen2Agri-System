#ifndef ARCHIVERMANAGER_H
#define ARCHIVERMANAGER_H
#include <QCoreApplication>
#include <QTimer>
#include <QMap>
#include <QStringList>
#include "persistencemanager_interface.h"


class ArchiverManager final : public QObject
{
    Q_OBJECT

public:
    ArchiverManager();
    ~ArchiverManager();

    void start(const QCoreApplication &app);

public slots:
    void lastAction();
    void exit();

private:
    typedef struct fileInfo {
        QString path;
        QString newPath;
        int processor;
        QDateTime timeToMove;
    } FILE_INFO;

    typedef struct configParam {
        QString rootPath;
        int maxAgeL2a;
        int maxAgeL3a;
        int maxAgeL3b;
        int maxAgeL4a;
        int maxAgeL4b;
    } CONFIG_PARAM;

    void loadConfigParam(const ConfigurationParameterValueList &configuration);
    void applyConfig();

    CONFIG_PARAM    readConfigParams;
    QMap<QString, QList<FILE_INFO>> files;
    OrgEsaSen2agriPersistenceManagerInterface clientInterface;
    QTimer mExitTimer;
};

#endif // ARCHIVERMANAGER_H
