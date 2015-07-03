#include <QApplication>
#include <QDebug>

#include "logger.hpp"
#include "maindialog.hpp"

int main(int argc, char *argv[])
{
    try {
        Logger::installMessageHandler();

        QApplication a(argc, argv);
        QApplication::setApplicationName(QStringLiteral("sen2agri-config"));

        registerMetaTypes();

        MainDialog w;
        w.show();

        return a.exec();
    } catch (const std::exception &e) {
        Logger::fatal(e.what());
    }
}
