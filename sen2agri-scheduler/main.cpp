#include <QCoreApplication>

#include "orchestrator_interface.h"
#include "dbusorchestratorproxy.hpp"
#include "databasetaskloader.hpp"
#include "schedulerapp.hpp"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    registerMetaTypes();

    DBusOrchestratorProxy orchestrator;
    DatabaseTaskLoader loader;
    SchedulerApp sapp(&loader, &orchestrator);
    sapp.StartRunning();

    return a.exec();
}
