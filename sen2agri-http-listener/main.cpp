#include <QCoreApplication>

#include <logger.hpp>
#include <httpserver/httplistener.h>
#include <httpserver/staticfilecontroller.h>

#include "requestmapper.h"

HttpListener *listener;
StaticFileController *staticFileController;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName("sen2agri-http-listener");

    Logger::installMessageHandler();

    QSettings fileSettings;
    fileSettings.setValue("path", "docroot");

    QSettings listenerSettings;
    listenerSettings.setValue("port", 8080);

    staticFileController = new StaticFileController(&fileSettings, &app);

    listener = new HttpListener(&listenerSettings, new RequestMapper(&app), &app);

    return app.exec();
}
