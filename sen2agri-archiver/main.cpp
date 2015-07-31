#include <QCoreApplication>

#include "logger.hpp"
#include "archivermanager.hpp"

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QCoreApplication app(argc, argv);

        registerMetaTypes();

        ArchiverManager arch;
        arch.start(app);
        return app.exec();
    } catch (const std::exception &e) {
        Logger::fatal(e.what());
    }
}
