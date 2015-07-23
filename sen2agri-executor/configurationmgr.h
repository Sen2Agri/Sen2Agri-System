#ifndef CONFIGURATIONMGR_H
#define CONFIGURATIONMGR_H

#include <QSettings>
#include <QVariantMap>

#include <string>
using namespace std;

class ConfigurationMgr
{
public:
    ConfigurationMgr();
    ~ConfigurationMgr();

    static ConfigurationMgr *GetInstance();

    bool GetValue(QString &strKey, QString &strVal, QString strDefVal = "");
    void SetValue(const QString &strKey, const QString &strVal);

private:
    QVariantMap m_mapVals;
};

#endif // CONFIGURATIONMGR_H
