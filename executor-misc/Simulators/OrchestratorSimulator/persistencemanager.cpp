#include <functional>

#include <QDebug>
#include <QDBusMessage>
#include <QThreadPool>

#include <iostream>
using namespace std;

#include "persistencemanager.h"

PersistenceManager::PersistenceManager(QObject *parent)
    : QObject(parent), m_strLastJobName("")
{
}


void PersistenceManager::Initialize(QString &strCfgPath)
{
    if(strCfgPath != "")
    {
        // If the ini file was provided, then use the configuration from here
        m_pSettings = new QSettings(strCfgPath, QSettings::IniFormat);
    }
}

ConfigurationSet PersistenceManager::GetConfigurationSet()
{
    return {};
}

ConfigurationParameterValueList
PersistenceManager::GetConfigurationParameters(const QString &prefix)
{
    cout << "----------------------------------------------------" << endl;
    cout << "PersistenceManagerSimulator: GetConfigurationParameters called!" << endl;
    cout << "PREFIX : " << prefix.toStdString().c_str() << endl;
    cout << "----------------------------------------------------" << endl;
    ConfigurationParameterValueList retList(
        {ConfigurationParameterValue("executor.listen_ip", 0, m_pSettings->value("SRV_IP_ADDR", "127.0.0.1").toString()),
         ConfigurationParameterValue("executor.listen_port", 0, m_pSettings->value("SRV_PORT_NO", "7777").toString()),
         ConfigurationParameterValue("executor.wrapper_path", 0, m_pSettings->value("PROCESSOR_WRAPPER_PATH", "./ProcessorWrapper").toString()),
         ConfigurationParameterValue("executor.processor.l2a.name", 0, m_pSettings->value("PROCESSOR_1_NAME", "CROP_TYPE").toString()),
         ConfigurationParameterValue("executor.processor.l2a.path", 0, m_pSettings->value("PROCESSOR_1_PATH", "./DummyProcessor").toString()),
         ConfigurationParameterValue("executor.processor.l3a.name", 0, "ATM_CORR"),
         ConfigurationParameterValue("executor.processor.l3a.path", 0, "atm_corrections.exe"),
         ConfigurationParameterValue("executor.processor.l3b.name", 0, "y"),
         ConfigurationParameterValue("executor.processor.l3b.path", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4a.name", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4a.path", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4b.name", 0, "y"),
         ConfigurationParameterValue("executor.processor.l4b.path", 0, "y")});

    return retList;
}

ConfigurationParameterValueList
PersistenceManager::GetJobConfigurationParameters(int jobId, const QString &prefix)
{
    return {};
}

KeyedMessageList
PersistenceManager::UpdateConfigurationParameters(const ConfigurationUpdateActionList &actions)
{
    return {};
}

KeyedMessageList PersistenceManager::UpdateJobConfigurationParameters(
    int jobId, const ConfigurationUpdateActionList &parameters)
{
    return {};
}

ProductToArchiveList PersistenceManager::GetProductsToArchive()
{
    return {};
}

KeyedMessageList PersistenceManager::UpdateProcessorStatus(const ProcessorStatusInfo &actions)
{
    cout << "----------------------------------------------------" << endl;
    cout << "OrchestratorSimulator: UpdateProcessorStatus called with " << actions.jobStatus.toStdString().c_str() << endl;
    cout << "JOB NAME : " << actions.jobName.toStdString().c_str() << endl;
    cout << "----------------------------------------------------" << endl;

    m_strLastJobName = actions.jobName;

    return {KeyedMessage("x", "y")};
}

QString& PersistenceManager::GetLastJobName()
{
    return m_strLastJobName;
}
