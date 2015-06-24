#include <iostream>
using namespace std;

#include "persistenceitfmodule.h"
#include "configurationmgr.h"

PersistenceItfModule::PersistenceItfModule() :
    clientInterface(OrgEsaSen2agriPersistenceManagerInterface::staticInterfaceName(),
                                                  QStringLiteral("/org/esa/sen2agri/persistencemanager"),
                                                  QDBusConnection::sessionBus())
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

void PersistenceItfModule::SendProcessorExecInfos(ProcessorExecutionInfos &execInfos)
{
    cout << "---------------------------------------------------------------" << endl;
    cout << "PersistenceItfModule : SendProcessorExecInfos called with status " << execInfos.strJobStatus.toStdString().c_str() << endl;
    cout << "---------------------------------------------------------------" << endl;
    ExecutionStatistics infos(execInfos.strJobNode,
                              0, /* exit code */
                              execInfos.strUserTime.toInt(),
                              execInfos.strSystemTime.toInt(),
                              execInfos.strExecutionDuration.toInt(),
                              execInfos.strMaxRss.toInt(),
                              execInfos.strMaxVmSize.toInt(),
                              execInfos.strDiskRead.toInt(),
                              execInfos.strDiskWrite.toInt());
//    ProcessorStatusInfo infos(execInfos.GetJobId(), execInfos.GetJobName(), execInfos.GetJobStatus(),
//                                  execInfos.GetStartTime(), execInfos.GetExecutionDuration(), execInfos.GetCpuTime(),
//                                  execInfos.GetAveVmSize(), execInfos.GetMaxVmSize());

//    KeyedMessageList ret = clientInterface.UpdateProcessorStatus(infos);
//    QList<KeyedMessage>::iterator i;
//    for (i = ret.begin(); i != ret.end(); i++) {
//        cout << (*i).key.toStdString().c_str() << "\n";
//        cout << (*i).text.toStdString().c_str() << "\n";
//    }
}

void PersistenceItfModule::RequestConfiguration()
{
    auto mainKeys = clientInterface.GetConfigurationParameters("executor.");
    connect(new QDBusPendingCallWatcher(mainKeys, this), &QDBusPendingCallWatcher::finished,
            [this, mainKeys]() {
                if (mainKeys.isValid()) {
                    SaveMainConfigKeys(mainKeys.value());
                } else if (mainKeys.isError()) {
                    qDebug() << mainKeys.error().message();
                }
    });

    auto processorsKeys = clientInterface.GetConfigurationParameters("executor.processor.");
    connect(new QDBusPendingCallWatcher(processorsKeys, this), &QDBusPendingCallWatcher::finished,
            [this, processorsKeys]() {
                if (processorsKeys.isValid()) {
                    SaveProcessorsConfigKeys(processorsKeys.value());
                } else if (processorsKeys.isError()) {
                    qDebug() << processorsKeys.error().message();
                }
    });
}

void PersistenceItfModule::SaveMainConfigKeys(const ConfigurationParameterValueList &configuration)
{
    for (const auto &p : configuration) {
        if(p.key == "executor.listen_ip")
        {
            QString strKey("SRV_IP_ADDR");
            ConfigurationMgr::GetInstance()->SetValue(strKey, p.value);
        }
        if(p.key == "executor.listen_port")
        {
            QString strKey("SRV_PORT_NO");
            ConfigurationMgr::GetInstance()->SetValue(strKey, p.value);
        }
        if(p.key == "executor.wrapper_path")
        {
            QString strKey("PROCESSOR_WRAPPER_PATH");
            ConfigurationMgr::GetInstance()->SetValue(strKey, p.value);
        }
    }
}

void PersistenceItfModule::SaveProcessorsConfigKeys(const ConfigurationParameterValueList &configuration)
{
    int nCurProc = 0;
    for (const auto &p : configuration) {
        QStringList listKeys = p.key.split('.');
        // normally, the first 2 should be "executor" and "processor"
        // we do not check them again
        if((listKeys.size() == 4) &&
                (listKeys.at(3).compare("name", Qt::CaseInsensitive) == 0)) {
            QString strProcessorName = listKeys.at(2);
            if(!strProcessorName.isEmpty() && !p.value.isEmpty())
            {
                QString strProcessorPath;
                if(GetProcessorPathForName(configuration, strProcessorName, strProcessorPath) &&
                        !strProcessorPath.isEmpty())
                {
                    QString strNameKey = QString("PROCESSOR_%1_NAME").arg(nCurProc+1);
                    ConfigurationMgr::GetInstance()->SetValue(strNameKey, p.value);
                    QString strPathKey = QString("PROCESSOR_%1_PATH").arg(nCurProc+1);
                    ConfigurationMgr::GetInstance()->SetValue(strPathKey, strProcessorPath);
                    nCurProc++;
                }
            }
        }
    }
    if(nCurProc > 0)
    {
        QString strKey = QString("PROCESSORS_NUMBER");
        QString strVal = QString::number(nCurProc);
        ConfigurationMgr::GetInstance()->SetValue(strKey, strVal);
    }

    emit OnConfigurationReceived();
}

bool PersistenceItfModule::GetProcessorPathForName(const ConfigurationParameterValueList &configuration,
                                          const QString &name, QString &path)
{
    QString strPathKey = QString("executor.processor.%1.path").arg(name);
    for (const auto &p : configuration) {
        if(p.key == strPathKey)
        {
            path = p.value;
            return true;
        }
    }

    return false;
}



