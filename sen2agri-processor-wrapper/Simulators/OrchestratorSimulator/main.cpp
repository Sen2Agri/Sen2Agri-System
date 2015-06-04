#include <QCoreApplication>
#include "ProcessorsExecutorProxy.h"
#include "ApplicationClosingListener.h"

#include <string>
#include <fstream>

using namespace std;

#define SERVICE_NAME "org.esa.sen2agri.processorsexecutor"

enum {PROCESSOR_ENDED = 1, PROCESSOR_INFO_MSG = 2, START_PROCESSOR_REQ = 3, STOP_PROCESSOR_REQ = 4};

QString strStartRequest("{                       \
   \"MSG_TYPE\":3,                          \
   \"PROC_NAME\" : \"CROP_TYPE\",           \
   \"PROC_ARGS\": \"crop args\"             \
}");

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    org::esa::sen2agri::processorsExecutor *pProcessorExecutor =
            new org::esa::sen2agri::processorsExecutor("org.esa.sen2agri.processorsexecutor",
                                            "/org/esa/sen2agri/processorsexecutor",
                                            QDBusConnection::sessionBus(), 0);
    qDebug() << "Result from 1# " << pProcessorExecutor->ExecuteProcessor(strStartRequest);


    QThread::sleep(2);

    string line;
    ifstream myfile ("../../logs/slurm_last_job_name.txt");
    if (myfile.is_open())
    {
        if(getline(myfile,line))
        {
            QString strLine(line.c_str());
            qDebug() << "Result from 2# " << pProcessorExecutor->StopProcessorJob(strLine);
        }
        myfile.close();
    }

    // Task parented to the application so that it
    // will be deleted by the application.
    ApplicationClosingListener *closingListener = new ApplicationClosingListener(&a);

    // This will cause the application to exit when
    // the closingListener signals finished.
    QObject::connect(closingListener, SIGNAL(finished()), &a, SLOT(quit()));

    // This will run the closingListener from the application event loop.
    QTimer::singleShot(0, closingListener, SLOT(run()));

    return a.exec();
}
