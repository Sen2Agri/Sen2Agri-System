#include <QCoreApplication>
#include <QDebug>
#include <QVariantMap>
#include <QTimer>

#include "logger.hpp"

#include "processorwrapper.h"
#include "applicationclosinglistener.h"
#include "simpleudpinfosclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Logger::debug(QString::number(argc));
    QStringList listParams;
    for(int i = 1; i < argc; i++) {
        Logger::debug(argv[i]);
        listParams.append(argv[i]);
    }

    // Task parented to the application so that it
    // will be deleted by the application.
    ProcessorWrapper procWrp;

    if(listParams.size() > 0)
    {
        if(procWrp.Initialize(listParams))
        {
            if(!procWrp.ExecuteProcessor())
            {
                Logger::error("Execution of the processor FAILED!");
            }
            else
            {
                Logger::info("Execution of the processor SUCEEDED!");
            }
        }
    }

    // Task parented to the application so that it
    // will be deleted by the application.
    ApplicationClosingListener *listener = new ApplicationClosingListener(&a);

    // This will cause the application to exit when
    // the task signals finished.
    QObject::connect(listener, SIGNAL(finished()), &a, SLOT(quit()));

    // This will run the task from the application event loop.
    QTimer::singleShot(0, listener, SLOT(run()));

    return a.exec();
}
