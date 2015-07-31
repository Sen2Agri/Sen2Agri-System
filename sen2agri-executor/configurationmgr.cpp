#include <QDir>
#include <QDebug>

#include <iostream>
using namespace std;

#include "configurationmgr.h"


ConfigurationMgr::ConfigurationMgr()
{
}

ConfigurationMgr::~ConfigurationMgr()
{
}

/*static*/
ConfigurationMgr *ConfigurationMgr::GetInstance()
{
    static ConfigurationMgr instance;
    return &instance;
}

bool ConfigurationMgr::GetValue(QString &strKey, QString &strVal, QString strDefVal)
{
    if (m_mapVals.contains(strKey))
    {
        strVal = m_mapVals.value(strKey).toString();
        if(strVal != "")
        {
            return true;
        }
    }

    strVal = strDefVal;

    return false;
}

void ConfigurationMgr::SetValue(const QString &strKey, const QString &strVal)
{
    // set the val for the given key
    // overwrite the existing value if the key exists
    m_mapVals[strKey] = strVal;
}

