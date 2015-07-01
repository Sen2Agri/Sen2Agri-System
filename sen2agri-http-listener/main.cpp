#include <QCoreApplication>

#include <httpserver/httplistener.h>
#include <httpserver/staticfilecontroller.h>

#include "logger.hpp"
#include "requestmapper.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("sen2agri-http-listener");

    Logger::installMessageHandler();

    QSettings fileSettings;
    fileSettings.setValue("path", "docroot");

    QSettings listenerSettings;
    listenerSettings.setValue("port", 8080);

    StaticFileController staticFileController(&fileSettings);

    RequestMapper mapper(staticFileController);
    HttpListener listener(&listenerSettings, &mapper);

    Q_UNUSED(listener);

    return app.exec();
}
