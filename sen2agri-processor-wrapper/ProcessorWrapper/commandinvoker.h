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

    bool InvokeCommand(QString &strCmd, bool bIsAsync);
    void StopCurCmdExec();
    QString& GetExecutionLog();
    void SetListener(ICommandInvokerListener *pListener);

private:
    QProcess m_process;
    QString m_logStr;
    ICommandInvokerListener *m_pListener;
};

#endif // COMMANDINVOKER_H
