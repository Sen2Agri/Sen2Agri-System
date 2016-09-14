#include <iostream>
using namespace std;

#include "persistenceitfmodule.h"
#include "configurationmgr.h"
#include "settings.hpp"
#include "configuration.hpp"

PersistenceItfModule::PersistenceItfModule()
    : clientInterface(Settings::readSettings(getConfigurationFile(*QCoreApplication::instance())))
{
}

PersistenceItfModule::~PersistenceItfModule()
{
}

/*static*/
PersistenceItfModule *PersistenceItfModule::GetInstance()
{
    static PersistenceItfModule instance;
    return &instance;
}

void PersistenceItfModule::MarkStepPendingStart(int taskId, QString &name)
{
    clientInterface.MarkStepPendingStart(taskId, name);
}

void PersistenceItfModule::MarkStepStarted(int taskId, QString &name)
{
    clientInterface.MarkStepStarted(taskId, name);
}

bool PersistenceItfModule::MarkStepFinished(int taskId,
                                            QString &name,
                                            ProcessorExecutionInfos &statistics)
{
    // Convert ProcessorExecutionInfos to ExecutionStatistics
    ExecutionStatistics newStats;
    newStats.diskReadBytes = statistics.strDiskRead.toLong();
    newStats.diskWriteBytes = statistics.strDiskWrite.toLong();
    newStats.durationMs = ParseTimeStr(statistics.strCpuTime);
    newStats.exitCode = statistics.strExitCode.toInt();
    newStats.maxRssKb = statistics.strMaxRss.toInt();
    newStats.maxVmSizeKb = statistics.strMaxVmSize.toInt();
    newStats.node = statistics.strJobNode;
    newStats.systemCpuMs = ParseTimeStr(statistics.strSystemTime);
    newStats.userCpuMs = ParseTimeStr(statistics.strUserTime);
    newStats.stdOutText = statistics.strStdOutText;
    newStats.stdErrText = statistics.strStdErrText;

    return clientInterface.MarkStepFinished(taskId, name, newStats);
}

void PersistenceItfModule::RequestConfiguration()
{
    try
    {
        const auto &keys = clientInterface.GetConfigurationParameters("executor.");
        SaveMainConfigKeys(keys);
        // SaveProcessorsConfigKeys(keys);
        emit OnConfigurationReceived();
    }
    catch (const std::exception &e)
    {
        qDebug() << e.what();
    }
}

QString PersistenceItfModule::GetExecutorQos(int processorId) {
    QString qos;
    const auto &strProcQosKey = QString("executor.processor.%1.slurm_qos")
            .arg(clientInterface.GetProcessorShortName(processorId));
    const auto &keys = clientInterface.GetConfigurationParameters(strProcQosKey);
    GetValueForKey(keys, strProcQosKey, qos);
    return qos;
}

QString PersistenceItfModule::GetExecutorPartition(int processorId) {
    QString partition;
    const auto &strProcPartKey = QString("executor.processor.%1.slurm_partition")
            .arg(clientInterface.GetProcessorShortName(processorId));
    const auto &keys = clientInterface.GetConfigurationParameters(strProcPartKey);
    GetValueForKey(keys, strProcPartKey, partition);
    return partition;
}

void PersistenceItfModule::SaveMainConfigKeys(const ConfigurationParameterValueList &configuration)
{
    for (const auto &p : configuration) {
        if (p.key == "executor.listen-ip") {
            QString strKey("SRV_IP_ADDR");
            ConfigurationMgr::GetInstance()->SetValue(strKey, p.value);
        }
        if (p.key == "executor.listen-port") {
            QString strKey("SRV_PORT_NO");
            ConfigurationMgr::GetInstance()->SetValue(strKey, p.value);
        }
        if (p.key == "executor.wrapper-path") {
            QString strKey("PROCESSOR_WRAPPER_PATH");
            ConfigurationMgr::GetInstance()->SetValue(strKey, p.value);
        }
    }
}

void
PersistenceItfModule::SaveProcessorsConfigKeys(const ConfigurationParameterValueList &configuration)
{
    int nCurProc = 0;
    for (const auto &p : configuration) {
        QStringList listKeys = p.key.split('.');
        // normally, the first 2 should be "executor" and "processor"
        // we do not check them again
        if ((listKeys.size() == 4) && (listKeys.at(3).compare("name", Qt::CaseInsensitive) == 0)) {
            QString strProcessorName = listKeys.at(2);
            if (!strProcessorName.isEmpty() && !p.value.isEmpty()) {
                QString strProcessorPath;
                if (GetProcessorPathForName(configuration, strProcessorName, strProcessorPath) &&
                    !strProcessorPath.isEmpty()) {
                    QString strNameKey = QString("PROCESSOR_%1_NAME").arg(nCurProc + 1);
                    ConfigurationMgr::GetInstance()->SetValue(strNameKey, p.value);
                    QString strPathKey = QString("PROCESSOR_%1_PATH").arg(nCurProc + 1);
                    ConfigurationMgr::GetInstance()->SetValue(strPathKey, strProcessorPath);
                    nCurProc++;
                }
            }
        }
    }
    if (nCurProc > 0) {
        QString strKey = QString("PROCESSORS_NUMBER");
        QString strVal = QString::number(nCurProc);
        ConfigurationMgr::GetInstance()->SetValue(strKey, strVal);
    }
}

bool PersistenceItfModule::GetProcessorPathForName(
    const ConfigurationParameterValueList &configuration, const QString &name, QString &path)
{
    const auto &strPathKey = QString("executor.processor.%1.path").arg(name);
    return GetValueForKey(configuration, strPathKey, path);
}

bool PersistenceItfModule::GetValueForKey(
    const ConfigurationParameterValueList &configuration, const QString &key, QString &value)
{
    for (const auto &p : configuration) {
        if (p.key == key) {
            value = p.value;
            return true;
        }
    }

    return false;
}

long PersistenceItfModule::ParseTimeStr(QString &strTime)
{
    // This function expects a string like [DD-[hh:]]mm:ss.mss
    QString strDays;
    QString strHours;
    QString strMinutes;
    QString strSeconds;
    QString strMillis;

    QStringList list = strTime.split(':');
    int listSize = list.size();
    QString firstElem = list.at(0);
    QStringList listDate = firstElem.split('-');
    if (listDate.size() > 1) {
        strDays = listDate.at(0);
        strHours = listDate.at(1);
        strMinutes = list.at(1);
        strSeconds = list.at(2);
    } else {
        // we have no separator for the days
        if (listSize == 3) {
            strHours = list.at(0);
            strMinutes = list.at(1);
            strSeconds = list.at(2);
        } else if (listSize == 2) {
            strMinutes = list.at(0);
            strSeconds = list.at(1);
        } else {
            // unknown format
            return -1;
        }
    }
    QStringList listSS = strSeconds.split('.');
    if (listSS.size() == 2) {
        strSeconds = listSS.at(0);
        strMillis = listSS.at(1);
    } else if (listSS.size() != 1) {
        // unknown format
        return -1;
    }
    long millis = (strDays.toLong() * 86400 + strHours.toLong() * 3600 + strMinutes.toLong() * 60 +
                   strSeconds.toLong()) *
                      1000 +
                  strMillis.toLong();
    return millis;
}
