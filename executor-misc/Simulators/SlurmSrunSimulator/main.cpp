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
    ofstream myfile1;
    myfile1.open("./logs.txt", std::ofstream::out | std::ofstream::app);
    myfile1 << "\n==================================================================" << "\n";
    myfile1 << "SlurmSimulator: Executed SLURM with the following parameters :" << "\n";
    for(int i = 1; i < argc; i++) {
        myfile1 << "SlurmSimulator:\t" << argv[i] << endl;
    }
    myfile1 << endl;

/*
    cout << "Executed SLURM with the following parameters :" << endl;

    for(int i = 1; i < argc; i++) {
        cout << "\t" << argv[i] << endl;
    }
*/
    if(argc >= 2)
    {
        if(QString::compare(argv[1], "--job-name", Qt::CaseInsensitive) == 0)
        {
            ofstream myfile;
            myfile.open ("./slurm_last_job_name.txt");
            myfile << argv[2] << "\n";
            myfile.close();

            for(int i = 3; i < argc; i++) {
                strCmd.append("\"").append(argv[i]).append("\" ");
            }

            CommandInvoker cmdInvoker;
            bool bRet = cmdInvoker.InvokeCommand(strCmd, false);
            myfile1 << "SlurmSimulator:\t InvokeCommand " << strCmd.toStdString().c_str() << " AND returned " << bRet << endl;
            myfile1 << "SlurmSimulator:\t returned " << cmdInvoker.GetExecutionLog().toStdString().c_str() << endl;

        } else if(strstr(argv[1], "--format=") != NULL)
        {
            // in this case we execute sstat

            /* The expected log is similar with the one below:

                JobID|JobName|NodeList|AveCPU|UserCPU|SystemCPU|ExitCode|AveVMSize|MaxRSS|MaxVMSize|MaxDiskRead|MaxDiskWrite
                3|ls|sen2agri-dev|00:00:00|00:00:00|00:00.002|0:0|0|0|0|0|0
                4|find|sen2agri-dev|00:00:00|00:00.297|00:00.413|1:0|0|0|0|0|0
                5|hostname|sen2agri-dev|00:00:00|00:00.005|00:00.008|0:0|0|0|0|0|0
                6|find|sen2agri-dev|00:00:00|00:00.299|00:00.344|1:0|0|0|0|0|0
                7|20150604144546391_CROP_TYPE|sen2agri-dev|00:00:00|00:00.004|00:00.003|0:0|0|0|0|0|0
                8|20150604155404295_CROP_TYPE|sen2agri-dev|00:00:00|00:00.003|00:00.004|0:0|0|0|0|0|0
                9|20150604160356666_CROP_TYPE|sen2agri-dev|00:00:00|00:00.001|00:00.006|0:0|0|0|0|0|0
                10|20150604160538979_CROP_TYPE|sen2agri-dev|00:00:00|00:00.001|00:00.006|0:0|0|0|0|0|0
                11|20150604160718384_CROP_TYPE|sen2agri-dev|00:00:00|00:00.002|00:00.004|0:0|0|0|0|0|0
                20|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00:00|00:00.001|2:0|0|0|0|0|0
                21|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00:00|00:00.001|2:0|0|0|0|0|0
                22|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.001|00:00:00|2:0|0|0|0|0|0
                23|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.001|00:00.005|0:0|0|0|0|0|0
                24|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.004|00:00.002|0:0|0|0|0|0|0
                25|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.001|00:00.006|0:0|0|0|0|0|0
                26|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.001|00:00.005|0:0|0|0|0|0|0
                27|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.004|00:00.002|0:0|0|0|0|0|0
                28|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00.004|00:00.003|0:0|0|0|0|0|0

                const char szJobIdStr[] = "jobid";
                const char szJobNameStr[] = "jobname";
                const char szCpuTimeStr[] = "avecpu";
                const char szAveVmSizeStr[] = "avevmsize";
                const char szMaxVmSizeStr[] = "maxvmsize";
            */

            cout << "JobID|JobName|NodeList|AveCPU|UserCPU|SystemCPU|ExitCode|AveVMSize|MaxRSS|MaxVMSize|MaxDiskRead|MaxDiskWrite" << endl;
            cout << "3|ls|sen2agri-dev|01:02:03|04:05:06.007|00:00.002|0:0|0|0|0|0|0" << endl;
            cout << "4|find|sen2agri-dev|22-11:12:13|22-14:15:16.297|00:00.413|1:0|0|0|0|0|0" << endl;
            cout << "5|hostname|sen2agri-dev|00:00|00.005|00:00.008|0:0|0|0|0|0|0" << endl;
            cout << "6|find|sen2agri-dev|00:00:00|00:00.299|00:00.344|1:0|0|0|0|0|0" << endl;
            cout << "7|20150604144546391_CROP_TYPE|sen2agri-dev|00:00:00|00:00.004|00:00.003|0:0|0|0|0|0|0" << endl;
            cout << "20|An_Unique_Step_ID|sen2agri-dev|00:00:00|00:00:00|00:00.001|2:0|0|0|0|0|0" << endl;

            string line;
            ifstream myfile ("./slurm_last_job_name.txt");
            if (myfile.is_open())
            {
                // add also the job names that we just executed before
                int nLineCnt = 0;
                while(getline(myfile,line))
                {
                  cout << "3|ls|sen2agri-dev|00:00:00|00:00:00|00:00.002|0:0|0|0|0|0|0" << endl;
                  cout << nLineCnt + 21 << "|" << line << "|sen2agri-dev|02-01:02:03.004|04:00:00|00:04.002|0:0|0|0"
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

    myfile1.close();

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
