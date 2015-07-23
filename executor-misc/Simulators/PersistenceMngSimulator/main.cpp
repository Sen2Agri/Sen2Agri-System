#include <QCoreApplication>
#include <QTimer>
#include "ApplicationClosingListener.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Task parented to the application so that it
    // will be deleted by the application.
    ApplicationClosingListener *closingListener = new ApplicationClosingListener(&a);
    //closingListener->SetPersistenceManager(&persistenceManager);

    // This will cause the application to exit when
    // the closingListener signals finished.
    QObject::connect(closingListener, SIGNAL(finished()), &a, SLOT(quit()));

    // This will run the closingListener from the application event loop.
    QTimer::singleShot(35000, closingListener, SLOT(run()));

    return a.exec();
}
