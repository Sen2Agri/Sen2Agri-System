#include <QCoreApplication>
#include "archivermanager.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    registerMetaTypes();

    ArchiverManager arch;
    arch.start(app);
    return app.exec();
}
