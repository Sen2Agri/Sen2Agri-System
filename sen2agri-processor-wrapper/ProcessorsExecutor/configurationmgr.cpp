#include "configurationmgr.h"
#include <QDir>
#include <QDebug>

#include <iostream>
using namespace std;

ConfigurationMgr *ConfigurationMgr::m_pInstance = NULL;

ConfigurationMgr::ConfigurationMgr()
{
}

ConfigurationMgr::~ConfigurationMgr()
{
    if(m_pSettings) {
        delete m_pSettings;
    }
}

/*static*/
bool ConfigurationMgr::Initialize(QString &strCfgPath)
{
    if(m_pInstance)
        return true;
    m_pInstance = new ConfigurationMgr();
    return m_pInstance->Init(strCfgPath);
}

/*static*/
ConfigurationMgr *ConfigurationMgr::GetInstance()
{
    return m_pInstance;
}

bool ConfigurationMgr::GetValue(QString &strKey, QString &strVal, QString strDefVal)
{
    strVal = m_pSettings->value(strKey, strDefVal).toString();
    if(strVal == "") {
        return false;
    }
    return true;
}

bool ConfigurationMgr::Init(QString &strCfgPath)
{
    // TODO: This configuration could be got from Persistence manager
    m_pSettings = new QSettings(strCfgPath, QSettings::IniFormat);

    return true;
}
