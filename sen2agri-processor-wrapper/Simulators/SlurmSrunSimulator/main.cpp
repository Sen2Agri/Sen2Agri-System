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

                Jobid      Jobname    Partition Account    AllocCPUS  State     ExitCode
                ---------- ---------- ---------- ---------- ------- ---------- --------
                3          sja_init   andy       acct1            1 COMPLETED         0
                4          sjaload    andy       acct1            2 COMPLETED         0
                5          sja_scr1   andy       acct1            1 COMPLETED         0
                6          sja_scr2   andy       acct1           18 COMPLETED         2
                7          sja_scr3   andy       acct1           18 COMPLETED         0
                8          sja_scr5   andy       acct1            2 COMPLETED         0
                9          sja_scr7   andy       acct1           90 COMPLETED         1
                10         endscript  andy       acct1          186 COMPLETED         0

                const char szJobIdStr[] = "jobid";
                const char szJobNameStr[] = "jobname";
                const char szCpuTimeStr[] = "avecpu";
                const char szAveVmSizeStr[] = "avevmsize";
                const char szMaxVmSizeStr[] = "maxvmsize";
            */

            cout << "        Jobid      Jobname    AveCPU     AveVMSize  MaxVmSize  State     ExitCode" << endl;
            cout << "        ---------- ---------- ---------- ---------- --------- ---------- --------" << endl;
            cout << "        1          Jobname1    30         100        50        State      ExitCode" << endl;
            cout << "        2          Jobname2    40         200        60        State      ExitCode" << endl;
            cout << "        3          Jobname3    60         500        90        State      ExitCode" << endl;
            cout << "        4          Jobname4    70         400        110       State      ExitCode" << endl;

            string line;
            ifstream myfile ("../../logs/slurm_last_job_name.txt");
            if (myfile.is_open())
            {
                int nLineCnt = 0;
                while(getline(myfile,line))
                {
                  cout << "        "<< nLineCnt + 5     << "          " << line
                       << "    "    << 80 + nLineCnt    << "         "  << 600 + nLineCnt
                       << "        " << 120 + nLineCnt  << "       State      ExitCode"
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
