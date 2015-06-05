#ifndef CONFIGURATIONMGR_H
#define CONFIGURATIONMGR_H

#include <QSettings>
#include <string>
using namespace std;

class ConfigurationMgr
{
public:
    ConfigurationMgr();
    ~ConfigurationMgr();

    static bool Initialize(QString &strCfgPath);
    static ConfigurationMgr *GetInstance();

    bool GetValue(QString &strKey, QString &strVal, QString strDefVal = "");

private:
    QSettings *m_pSettings;
    static ConfigurationMgr *m_pInstance;
    bool Init(QString &strCfgPath);
};

#endif // CONFIGURATIONMGR_H
