#ifndef REQUESTPARAMSEXECUTIONINFOS_H
#define REQUESTPARAMSEXECUTIONINFOS_H

#include "requestparamsbase.h"

class RequestParamsExecutionInfos : public RequestParamsBase
{
public:
    RequestParamsExecutionInfos();
    bool ParseMessage(const QByteArray &message);

    bool IsExecutionStarted();
    bool IsExecutionEnded();
    bool IsLogMsg();

    const QString &GetJobName();
    const QString GetLogMsg();
    const QString GetExecutionTime();

private:
    QVariantMap m_msgVals;
    QString m_strJobName;
    QString m_strMsgType;
};

#endif // REQUESTPARAMSEXECUTIONINFOS_H
