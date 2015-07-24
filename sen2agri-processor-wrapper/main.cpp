#include <QCoreApplication>
#include <QDebug>
#include <QVariantMap>
#include <QTimer>

#include "processorwrapper.h"
#include "applicationclosinglistener.h"
#include "simpleudpinfosclient.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qDebug() << argc;
    QStringList listParams;
    for(int i = 1; i < argc; i++) {
        qDebug() << argv[i];
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
                qDebug() << "Execution of the processor FAILED!";
            }
            else
            {
                qDebug() << "Execution of the processor SUCEEDED!";
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
