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
    QVariantMap mapArgs;
    for(int i = 1; i < argc; i++) {
        qDebug() << argv[i];
        QString curArg(argv[i]);
        int nArgLen = curArg.length();
        int nEqPos = curArg.indexOf("=");
        if(nEqPos > 0 && nEqPos < (nArgLen-1)) {
            QString strKey = curArg.left(nEqPos);
            QString strVal = curArg.right(nArgLen-nEqPos-1);
            if(strKey.length() > 0 && strVal.length() > 0) {
                mapArgs[strKey] = strVal;
            }
        }
    }

    // Task parented to the application so that it
    // will be deleted by the application.
    ProcessorWrapper procWrp;

    if(mapArgs.size() > 0)
    {
        if(procWrp.Initialize(mapArgs))
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
