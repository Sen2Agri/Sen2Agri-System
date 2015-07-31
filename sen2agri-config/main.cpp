#include <QApplication>
#include <QDebug>

#include "logger.hpp"
#include "maindialog.hpp"

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QApplication app(argc, argv);

        registerMetaTypes();

        MainDialog w;
        w.show();

        return app.exec();
    } catch (const std::exception &e) {
        Logger::fatal(e.what());
    }
}
