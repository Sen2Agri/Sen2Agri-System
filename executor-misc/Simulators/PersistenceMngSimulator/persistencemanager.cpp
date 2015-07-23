#include <iostream>
using namespace std;
#include <functional>

#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QThreadPool>
#include <QSettings>
#include "persistencemanager.h"

PersistenceManager::PersistenceManager(QObject *parent)
    : QObject(parent)
{
}

ConfigurationSet PersistenceManager::GetConfigurationSet()
{
    return {};
}

ConfigurationParameterValueList PersistenceManager::GetConfigurationParameters(QString prefix)
{
    QSettings settings ("./persistence.ini", QSettings::IniFormat);
    QString strIp = settings.value("SRV_IP", "127.0.0.1").toString();
    QString strPort = settings.value("PORT_NO", "7777").toString();
    QString strWrapperPath = settings.value("WRAPPER_PATH", "./sen2agri-processor-wrapper").toString();

    cout << "----------------------------------------------------" << endl;
    cout << "PersistenceManagerSimulator: GetConfigurationParameters called!" << endl;
    cout << "PREFIX : " << prefix.toStdString().c_str() << endl;
    cout << "----------------------------------------------------" << endl;
    ConfigurationParameterValueList retList(
        {ConfigurationParameterValue("executor.listen_ip", 0, strIp),
         ConfigurationParameterValue("executor.listen_port", 0, strPort),
         ConfigurationParameterValue("executor.wrapper_path", 0, strWrapperPath)
/*         ConfigurationParameterValue("executor.processor.l2a.name", 0, m_pSettings->value("PROCESSOR_1_NAME", "CROP_TYPE").toString()),
         ConfigurationParameterValue("executor.processor.l2a.path", 0, m_pSettings->value("PROCESSOR_1_PATH", "./DummyProcessor").toString()),
         ConfigurationParameterValue("executor.processor.l3a.name", 0, "ATM_CORR"),
         ConfigurationParameterValue("executor.processor.l3a.path", 0, "atm_corrections.exe"),
         ConfigurationParameterValue("executor.processor.l3b.name", 0, "y"),
         ConfigurationParameterValue("executor.processor.l3b.path", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4a.name", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4a.path", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4b.name", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4b.path", 0, "y")*/});

    return retList;
}

JobConfigurationParameterValueList PersistenceManager::GetJobConfigurationParameters(int jobId,
                                                                                     QString prefix)
{
    return {};
}

KeyedMessageList
PersistenceManager::UpdateConfigurationParameters(ConfigurationUpdateActionList parameters)
{
    return {};
}

KeyedMessageList
PersistenceManager::UpdateJobConfigurationParameters(int jobId,
                                                     JobConfigurationUpdateActionList parameters)
{
    return {};
}

ProductToArchiveList PersistenceManager::GetProductsToArchive()
{
    return {};
}

void PersistenceManager::MarkProductsArchived(ArchivedProductList products)
{
}

int PersistenceManager::SubmitJob(NewJob job)
{
    return {};
}

int PersistenceManager::SubmitTask(NewTask task)
{
    return {};
}

void PersistenceManager::SubmitSteps(int taskId, NewStepList steps)
{
}

void PersistenceManager::MarkStepPendingStart(int taskId, QString name)
{
    cout << "----------------------------------------------------" << endl;
    cout << "PersistenceManager: MarkStepPendingStart called with task id " << taskId << endl;
    cout << "STEP NAME : " << name.toStdString().c_str() << endl;
    cout << "----------------------------------------------------" << endl;
}

void PersistenceManager::MarkStepStarted(int taskId, QString name)
{
    cout << "----------------------------------------------------" << endl;
    cout << "PersistenceManager: MarkStepStarted called with task id " << taskId << endl;
    cout << "STEP NAME : " << name.toStdString().c_str() << endl;
    cout << "----------------------------------------------------" << endl;
}

bool PersistenceManager::MarkStepFinished(int taskId, QString name, ExecutionStatistics statistics)
{
    cout << "----------------------------------------------------" << endl;
    cout << "PersistenceManager: MarkStepFinished called with task id " << taskId << endl;
    cout << "STEP NAME : " << name.toStdString().c_str() << endl;
    cout << "diskReadBytes: " << statistics.diskReadBytes << endl;
    cout << "diskWriteBytes: " << statistics.diskWriteBytes << endl;
    cout << "durationMs: " << statistics.durationMs << endl;
    cout << "exitCode: " << statistics.exitCode << endl;
    cout << "maxRssKb: " << statistics.maxRssKb << endl;
    cout << "maxVmSizeKb: " << statistics.maxVmSizeKb << endl;
    cout << "node: " << statistics.node.toStdString().c_str() << endl;
    cout << "systemCpuMs: " << statistics.systemCpuMs << endl;
    cout << "userCpuMs: " << statistics.userCpuMs << endl;
    cout << "----------------------------------------------------" << endl;

    return {};
}

void PersistenceManager::MarkJobPaused(int jobId)
{
}

void PersistenceManager::MarkJobResumed(int jobId)
{
}

void PersistenceManager::MarkJobCancelled(int jobId)
{
}

void PersistenceManager::MarkJobFinished(int jobId)
{
}

void PersistenceManager::MarkJobFailed(int jobId)
{
}

void PersistenceManager::MarkJobNeedsInput(int jobId)
{
}

TaskIdList PersistenceManager::GetJobTasksByStatus(int jobId, ExecutionStatusList statusList)
{
    return {};
}

JobStepToRunList PersistenceManager::GetTaskStepsForStart(int taskId)
{
    return {};
}

JobStepToRunList PersistenceManager::GetJobStepsForResume(int jobId)
{
    return {};
}

void PersistenceManager::InsertTaskFinishedEvent(TaskFinishedEvent event)
{
}

void PersistenceManager::InsertProductAvailableEvent(ProductAvailableEvent event)
{
}

void PersistenceManager::InsertJobCancelledEvent(JobCancelledEvent event)
{
}

void PersistenceManager::InsertJobPausedEvent(JobPausedEvent event)
{
}

void PersistenceManager::InsertJobResumedEvent(JobResumedEvent event)
{
}

void PersistenceManager::InsertJobSubmittedEvent(JobSubmittedEvent event)
{
}

UnprocessedEventList PersistenceManager::GetNewEvents()
{
    return {};
}

void PersistenceManager::MarkEventProcessingStarted(int eventId)
{
}

void PersistenceManager::MarkEventProcessingComplete(int eventId)
{
}

void PersistenceManager::InsertNodeStatistics(NodeStatistics statistics)
{
}

QString PersistenceManager::GetDashboardData(QDate since)
{
    return {};
}

