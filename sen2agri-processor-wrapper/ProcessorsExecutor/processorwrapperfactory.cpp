#include "processorwrapperfactory.h"
#include "configurationmgr.h"
#include "logger.h"

ProcessorWrapperFactory::ProcessorWrapperFactory()
{
    // get the name, path and description of each processor from the configuration
    QString strName;
    QString strPath;
    QString strDescr;
    QString strNameKey;
    QString strPathKey;
    QString strDescrKey;

    QString strVal;
    QString strKey("PROCESSORS_NUMBER");

    bool bOk = ConfigurationMgr::GetInstance()->GetValue(strKey, strVal);
    int nProcsCnt = strVal.toInt();
    if(!bOk || nProcsCnt == 0) {
        Logger::GetInstance()->error("No processor was defined in configuration (key PROCESSORS_NUMBER was not defined or is 0)!!!");
    }
    for(int i = 1; i < nProcsCnt+1; i++) {
        bOk = true;
        strNameKey = QString("PROCESSOR_%1_NAME").arg(i);
        bOk = ConfigurationMgr::GetInstance()->GetValue(strNameKey, strName);
        strPathKey = QString("PROCESSOR_%1_PATH").arg(i);
        bOk &= ConfigurationMgr::GetInstance()->GetValue(strPathKey, strPath);
        strDescrKey = QString("PROCESSOR_%1_DESCR").arg(i);
        // the description might be missing that is why we will not use the return for it
        ConfigurationMgr::GetInstance()->GetValue(strDescrKey, strDescr);
        if(bOk) {
            Register(strName, strPath, strDescr);
        } else {
            Logger::GetInstance()->error("Error loading the configuration for processor with index %d!!!", i);
        }
    }
}

ProcessorWrapperFactory::~ProcessorWrapperFactory()
{
}

/* static */
ProcessorWrapperFactory *ProcessorWrapperFactory::GetInstance()
{
    static ProcessorWrapperFactory instance;
    return &instance;
}

void ProcessorWrapperFactory::Register(QString &procName, QString &procCmd, QString& descr)
{
    ProcessorInfos infos(procName, procCmd, descr);
    m_ProcessorsInfosMap[procName] = infos;
}

bool ProcessorWrapperFactory::GetProcessorPath(QString &processorName, QString &retPath)
{
    ProcessorsInfosMap::iterator it = m_ProcessorsInfosMap.find(processorName);
    if( it != m_ProcessorsInfosMap.end() ) {
        retPath = it->second.command;
        return true;
    }
    return false;
}

bool ProcessorWrapperFactory::GetProcessorDescription(QString &processorName, QString &retDescr)
{
    ProcessorsInfosMap::iterator it = m_ProcessorsInfosMap.find(processorName);
    if( it != m_ProcessorsInfosMap.end() ) {
        retDescr = it->second.description;
        return true;
    }
    return false;
}
