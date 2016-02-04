#include <QTextStream>
#include <signal.h>

#include "archivermanager.hpp"
#include "settings.hpp"
#include "configuration.hpp"

ArchiverManager::ArchiverManager()
    : clientInterface(Settings::readSettings(getConfigurationFile(*QCoreApplication::instance())))
{
}

ArchiverManager::~ArchiverManager()
{
}

void ArchiverManager::start(const QCoreApplication &app)
{
    connect(&app, SIGNAL(aboutToQuit()), this, SLOT(lastAction()));
    connect(this, SIGNAL(exitLocal()), &app, SLOT(quit()), Qt::QueuedConnection);
    connect(&mExitTimer, SIGNAL(timeout()), this, SLOT(exit()));
    signal(SIGINT, &ArchiverManager::signalHandler);
    signal(SIGTERM, &ArchiverManager::signalHandler);

    try
    {
        const auto &products = clientInterface.GetProductsToArchive();
        productsToBeArchived(products);
    }
    catch (const std::exception &e)
    {
        qDebug() << e.what();
    }
}

void ArchiverManager::productsToBeArchived(const ProductToArchiveList &products)
{
    mFilesToBeDeleted.empty();
    ArchivedProductList archivedProducts;
    for (const auto &product : products) {
        if (product.archivePath.isEmpty()) {
            // TODO: add error in log
            qDebug() << "The archive path for product ID " << QString::number(product.productId)
                     << " is empty ";
            QTextStream(stdout) << "The archive path for product ID "
                                << QString::number(product.productId) << " is empty " << endl;
            continue;
        }
        if (product.currentPath.isEmpty()) {
            // TODO: add error in log
            qDebug() << "The current path for product ID " << QString::number(product.productId)
                     << " is empty ";
            QTextStream(stdout) << "The current path for product ID "
                                << QString::number(product.productId) << " is empty " << endl;
            continue;
        }
        QString archivePathBase(product.archivePath);
        if (product.archivePath.startsWith("/"))
            archivePathBase = product.archivePath.mid(1, product.archivePath.length() - 1);

        QDir archiveRootPath("/");
        if (!QDir("/" + archivePathBase).exists() && !archiveRootPath.mkpath(archivePathBase)) {
            // TODO: add error in log
            qDebug() << "Error in creating path for archiving "
                     << "/" + archivePathBase;
            QTextStream(stdout) << "Error in creating path for archiving "
                                << "/" + archivePathBase << endl;
            continue;
        }
        archiveRootPath.setPath("/" + archivePathBase);
        archiveRootPath.makeAbsolute();
        if (!archiveRootPath.exists()) {
            // TODO: add error in log
            qDebug() << "Error in setting path for archiving "
                     << "/" + archivePathBase;
            QTextStream(stdout) << "Error in setting path for archiving "
                                << "/" + archivePathBase << endl;
            continue;
        }

        QDirIterator itDir(product.currentPath, QDirIterator::Subdirectories);
        bool bDeleteCurrentPath = true;
        while (itDir.hasNext()) {
            itDir.next();
            QFileInfo prodFile(itDir.filePath());
            if (prodFile.isFile()) {
                QString intermediaryPath(prodFile.absolutePath());
                intermediaryPath.remove(
                    product.currentPath.endsWith("/")
                        ? product.currentPath.mid(0, product.currentPath.length() - 1)
                        : product.currentPath);
                if (intermediaryPath.length() > 0 && !intermediaryPath.startsWith("/"))
                    intermediaryPath.insert(0, '/');
                if (!QDir(archiveRootPath.absolutePath() + intermediaryPath).exists() &&
                    intermediaryPath.length() > 0 &&
                    !archiveRootPath.mkpath(intermediaryPath.mid(
                         1, intermediaryPath.length()))) // eliminate the first /, otherwise qt will
                                                         // not create the path
                {
                    // TODO: add error in log
                    qDebug() << "Error in creating path for archiving "
                             << archiveRootPath.absolutePath() + intermediaryPath;
                    QTextStream(stdout) << "Error in creating path for archiving "
                                        << archiveRootPath.absolutePath() + intermediaryPath
                                        << endl;
                    bDeleteCurrentPath = false;
                    continue;
                }
                QFile fileToBeCopied(prodFile.absoluteFilePath());
                QString archivedFileName =
                    archiveRootPath.absolutePath() + intermediaryPath + "/" + prodFile.fileName();
                if (QFile(archivedFileName).exists() && !QFile(archivedFileName).remove()) {
                    // TODO: add error in log
                    qDebug() << "Error in deleting the already existing file in archive path "
                             << archivedFileName;
                    QTextStream(stdout)
                        << "Error in deleting the already existing file in archive path "
                        << archivedFileName << endl;
                    bDeleteCurrentPath = false;
                    continue;
                }
                if (!fileToBeCopied.copy(archivedFileName)) {
                    // TODO: add error in log
                    qDebug() << "Error in copying file " << fileToBeCopied.fileName() << " to "
                             << archivedFileName;
                    QTextStream(stdout) << "Error in copying file " << fileToBeCopied.fileName()
                                        << " to " << archivedFileName << endl;
                    bDeleteCurrentPath = false;
                } else
                    archivedProducts.append(
                        ArchivedProduct(product.productId, product.archivePath));
            }
        }
        if (bDeleteCurrentPath)
            mFilesToBeDeleted.append(product.currentPath);
    }

    if (!archivedProducts.isEmpty()) {
        try
        {
            clientInterface.MarkProductsArchived(ArchivedProductList()); // archivedProducts
            deleteFiles();
        }
        catch (const std::exception &e)
        {
            qDebug() << e.what();
        }
    } else
        emit exitLocal();
}

void ArchiverManager::deleteFiles()
{
    for (const auto &pathToBeDeleted : mFilesToBeDeleted) {
        QDir dirDel(pathToBeDeleted);
        qDebug() << "Path to be deleted: " << pathToBeDeleted;
        if (!dirDel.removeRecursively()) {
            // TODO: add error in log
            qDebug() << "Error in recursively deleting path " << pathToBeDeleted;
            QTextStream(stdout) << "Error in recursively deleting path " << pathToBeDeleted << endl;
        }
    }
    emit exitLocal();
}

void ArchiverManager::signalHandler(int signal)
{
    qDebug() << "Signal " << QString::number(signal) << " caught";
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
