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

    const QString &GetJobName() const;
    const QString &GetLogMsg() const;
    const QString &GetExecutionTime() const;
    const QString &GetStdOutText() const;
    const QString &GetStdErrText() const;
    const QString &GetStatusText() const;
    int GetExitCode() const;

private:
    QString m_strJobName;
    QString m_strMsgType;
    QString m_strStatusText;
    int m_nExitCode;
    // no longer used
    QString m_strLogMsg;
    QString m_strExecTime;
    QString m_strStdOutText;
    QString m_strStdErrText;
};

#endif // REQUESTPARAMSEXECUTIONINFOS_H
