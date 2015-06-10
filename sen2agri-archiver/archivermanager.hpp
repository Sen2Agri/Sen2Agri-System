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
signals:
    void exitLocal();

public slots:
    void lastAction();
    static void exit();

private:
    static void signalHandler(int signal);
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

    void productsToBeArchived(const ProductToArchiveList &products);
    void deleteFiles();

    QStringList mFilesToBeDeleted;
    //QList<Product> products;

    CONFIG_PARAM    readConfigParams;
    QMap<QString, QList<FILE_INFO>> files;
    OrgEsaSen2agriPersistenceManagerInterface clientInterface;
    QTimer mExitTimer;
};

#endif // ARCHIVERMANAGER_H
