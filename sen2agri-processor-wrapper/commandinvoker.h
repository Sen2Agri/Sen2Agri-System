#ifndef COMMANDINVOKER_H
#define COMMANDINVOKER_H


#include <QProcess>
#include <QString>
#include "icommandinvokerlistener.h"

class CommandInvoker
{
public:
    CommandInvoker();
    ~CommandInvoker();

    bool InvokeCommand(QString &strCmd, QStringList &listParams, bool bIsAsync, int &exitCode);
    void StopCurCmdExec();
    const QString& GetStandardOutputLog() const;
    const QString& GetStandardErrorLog() const;
    void SetListener(ICommandInvokerListener *pListener);

private:
    QProcess m_process;
    QString m_stdOutText;
    QString m_stdErrText;
    ICommandInvokerListener *m_pListener;
};

#endif // COMMANDINVOKER_H
