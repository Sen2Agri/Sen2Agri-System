#include "ApplicationClosingListener.h"

#include <string>
#include <fstream>
#include <iostream>
using namespace std;

QString strStartRequest("{                 \
   \"PROC_NAME\" : \"CROP_TYPE\",          \
   \"PROC_ARGS\": \"crop args\",           \
   \"STEP_ID\": \"An_Unique_Step_ID\"      \
}");


ApplicationClosingListener::ApplicationClosingListener(QObject *parent) : QObject(parent)
{
    m_pProcessorExecutor =
            new org::esa::sen2agri::processorsExecutor("org.esa.sen2agri.processorsexecutor",
                                            "/org/esa/sen2agri/processorsexecutor",
                                            QDBusConnection::sessionBus(), 0);
}

void ApplicationClosingListener::HandleSendExecuteProcessor()
{
    qDebug() << "Result from 1# " << m_pProcessorExecutor->ExecuteProcessor(strStartRequest);
}

void ApplicationClosingListener::HandleSendCancelProcessor()
{
    QString strJobName = m_pPersistenceMng->GetLastJobName();
    if(!strJobName.isEmpty())
    {
        qDebug() << "Result from 2# " << m_pProcessorExecutor->StopProcessorJob(strJobName);
    }
    else
    {
        string line;
        ifstream myfile ("../../logs/slurm_last_job_name.txt");
        if (myfile.is_open())
        {
            if(getline(myfile,line))
            {
                QString strLine(line.c_str());
                qDebug() << "Result from 2# " << m_pProcessorExecutor->StopProcessorJob(strLine);
            }
            myfile.close();
        }
    }
}
