#include <QCoreApplication>

#include "logger.hpp"
#include "archivermanager.hpp"

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QCoreApplication app(argc, argv);
        QCoreApplication::setApplicationName(QStringLiteral("sen2agri-archiver"));

        registerMetaTypes();

        ArchiverManager arch;
        arch.start(app);
        return app.exec();
    } catch (const std::exception &e) {
        Logger::fatal(e.what());
    }
}
