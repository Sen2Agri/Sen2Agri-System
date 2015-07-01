#include <QCoreApplication>
#include <QTimer>

#include <iostream>
#include <fstream>
using namespace std;

#include "commandinvoker.h"
#include "applicationclosinglistener.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    QString strCmd;

    // the first is the srun simulator exe,
    // the second is --job-name or --format
    // the third is the value for the job-name
/*    cout << "Executed SLURM with the following parameters :" << endl;

    for(int i = 1; i < argc; i++) {
        cout << "\t" << argv[i] << endl;
    }
*/
    if(argc >= 2)
    {
        if(QString::compare(argv[1], "--job-name", Qt::CaseInsensitive) == 0)
        {
            ofstream myfile;
            myfile.open ("../../logs/slurm_last_job_name.txt");
            myfile << argv[2] << "\n";
            myfile.close();

            for(int i = 3; i < argc; i++) {
                strCmd.append(argv[i]).append(" ");
            }

            CommandInvoker cmdInvoker;
            cmdInvoker.InvokeCommand(strCmd, false);

        } else if(strstr(argv[1], "--format=") != NULL)
        {
            // in this case we execute sstat

            /* The expected log is similar with the one below:

                JobID|JobName|AveCPU|AveVMSize|MaxVMSize
                2|ls|00:00:00|0|0
                3|ls|00:00:00|0|0
                4|find|00:00:00|0|0
                5|hostname|00:00:00|0|0
                6|find|00:00:00|0|0
                7|20150604144546391_CROP_TYPE|00:00:00|0|0

                const char szJobIdStr[] = "jobid";
                const char szJobNameStr[] = "jobname";
                const char szCpuTimeStr[] = "avecpu";
                const char szAveVmSizeStr[] = "avevmsize";
                const char szMaxVmSizeStr[] = "maxvmsize";
            */

            cout << "JobID|JobName|AveCPU|AveVMSize|MaxVMSize" << endl;
            cout << "2|ls|00:00:00|0|0" << endl;
            cout << "3|ls|00:00:00|0|0" << endl;
            cout << "4|find|00:00:00|0|0" << endl;
            cout << "5|hostname|00:00:00|0|0" << endl;
            cout << "6|find|00:00:00|0|0" << endl;
            cout << "7|20150604144546391_CROP_TYPE|00:00:00|0|0" << endl;

            string line;
            ifstream myfile ("../../logs/slurm_last_job_name.txt");
            if (myfile.is_open())
            {
                int nLineCnt = 0;
                while(getline(myfile,line))
                {
                  cout << nLineCnt + 8 << "|" << line
                       << "|"<< 80 + nLineCnt << "|"  << 600 + nLineCnt
                       << "|" << 120 + nLineCnt
                       << endl;

                  nLineCnt++;
                }
                myfile.close();
            }
            else cout << "Unable to open file";
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
