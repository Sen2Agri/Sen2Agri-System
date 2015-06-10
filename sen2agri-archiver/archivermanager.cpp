#include "archivermanager.hpp"
#include <QTextStream>
#include <signal.h>

ArchiverManager::ArchiverManager() :
    clientInterface(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                                                  QStringLiteral("/"),
                                                  QDBusConnection::systemBus())
{


}

ArchiverManager::~ArchiverManager()
{

}

void ArchiverManager::start(const QCoreApplication& app)
{
    connect(&app, SIGNAL(aboutToQuit()), this, SLOT(lastAction()));
    connect(this, SIGNAL(exitLocal()), &app, SLOT(quit()) , Qt::QueuedConnection);
    connect(&mExitTimer, SIGNAL(timeout()), this, SLOT(exit()));
    signal(SIGINT, &ArchiverManager::signalHandler);
    signal(SIGTERM, &ArchiverManager::signalHandler);
    //TODO: take parameters
    //TODO create for each site a FILE_INFO obj insert it into files

    //TODO: split in groups based on sites
    //
    /*
    auto promise = clientInterface.GetProductsToArchive();
    connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
            [this, promise]() {
                if (promise.isValid()) {
                    productsToBeArchived(promise.value());
                } else if (promise.isError()) {
                    qDebug() << promise.error().message();
                }
            });
            */
    /* DEBUG ONLY: to be removed*/
    ProductToArchiveList testList;
    QString currPath = "/home/miradmin/prj/sen2agri/tests/test/";
    QString archPath = "/home/miradmin/prj/sen2agri/tests/";
    char sites[10][20] = {"africa/", "africa/", "africa/", "africa/", "europe/", "europe/", "australia/", "australia/", "australia/", "australia/"};
    char products[10][20] = {"l2a/", "l3a/", "l3b/", "l4b/", "l2a/", "l3b/", "l3b/", "l2a/", "l4a/", "l4b/"};
    for(auto i = 0; i< 10; i++) {
        testList.append(ProductToArchive(i, currPath + "prod_" + QString::number(i) , archPath + sites[i] + products[i]));
    }
    productsToBeArchived(testList);
    /* END OF DEBUG ONLY*/
}

void ArchiverManager::productsToBeArchived(const ProductToArchiveList &products)
{
    mFilesToBeDeleted.empty();
    ArchivedProductList archivedProducts;
    for (const auto &product : products)
    {
        QDir archiveRootPath;

        if(product.archivePath.isEmpty())
        {
            //TODO: add error in log
            qDebug() << "The archive path for product ID " << QString::number(product.productId) << " is empty ";
            continue;
        }
        QString archivePathBase(product.archivePath);
        if(product.archivePath.startsWith("/"))
            archivePathBase = product.archivePath.mid(1, product.archivePath.length() - 1);

        archiveRootPath.setCurrent("/");
        if(!QDir("/" + archivePathBase).exists() && !archiveRootPath.mkpath(archivePathBase)) {
            //TODO: add error in log
            qDebug() << "Error in creating path for archiving " << "/" + archivePathBase;
            continue;
        }
        archiveRootPath.setCurrent("/" + archivePathBase);
        archiveRootPath.makeAbsolute();

        //QString currentPathBase(product.currentPath);
        QDirIterator itDir(product.currentPath, QDirIterator::Subdirectories);
        while(itDir.hasNext())
        {
            itDir.next();
            QFileInfo prodFile(itDir.filePath());
            if (prodFile.isFile())
            {
                QString intermediaryPath(prodFile.absolutePath());
                intermediaryPath.remove(product.currentPath);
                if(!QDir(archiveRootPath.absolutePath() + intermediaryPath).exists() && intermediaryPath.length() > 0 &&
                    !archiveRootPath.mkpath(intermediaryPath.mid(1, intermediaryPath.length()))) //eliminate the first /, otherwise qt will not create the path
                {
                        //TODO: add error in log
                        qDebug() << "Error in creating path for archiving " << archiveRootPath.absolutePath() + intermediaryPath;
                        continue;
                }
                QFile fileToBeCopied(prodFile.absoluteFilePath());
                QString archivedFileName = archiveRootPath.absolutePath() +
                        intermediaryPath +
                        "/" +
                        prodFile.fileName();
                if(QFile(archivedFileName).exists() &&
                    !QFile(archivedFileName).remove())
                {
                        //TODO: add error in log
                        qDebug() << "Error in deleting the already existing file in archive path " << archivedFileName;
                        continue;
                }
                if(!fileToBeCopied.copy(archivedFileName) ) {
                    //TODO: add error in log
                    qDebug() << "Error in copying file " << fileToBeCopied.fileName() << " to " << archivedFileName;
                }
                else {
                    mFilesToBeDeleted.append(fileToBeCopied.fileName());
                    archivedProducts.append(ArchivedProduct(product.productId, product.archivePath));
                }
            }
        }
    }

    if(!archivedProducts.isEmpty()) {
        auto promise = clientInterface.MarkProductsArchived(archivedProducts);
        connect(new QDBusPendingCallWatcher(promise, this), &QDBusPendingCallWatcher::finished,
                [this, promise]() {
                    qDebug() << "test " << endl;
                    if (promise.isValid()) {
                        deleteFiles();
                    } else if (promise.isError()) {
                        qDebug() << promise.error().message();
                    }
                });
    }
    //QCoreApplication::quit();
    //emit exitLocal();
}

void ArchiverManager::deleteFiles()
{
    for(const auto fileToBeDeleted : mFilesToBeDeleted) {
        if(!QFile::remove(fileToBeDeleted))
          qDebug() << "Error in deleting file " << fileToBeDeleted;
    }
    mExitTimer.start(100);
}

void ArchiverManager::signalHandler(int signal)
{
    qDebug() << "Signal " << QString::number(signal) << " caught" << endl;
    QCoreApplication::quit();
}

void ArchiverManager::lastAction()
{
    QTextStream(stdout) << "BYE\n";
}

void ArchiverManager::exit()
{
    QCoreApplication::exit(0);
}
