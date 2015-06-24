#ifndef COMMANDINVOKER_H
#define COMMANDINVOKER_H


#include <QProcess>
#include <QString>

class CommandInvoker
{
public:
    CommandInvoker();
    ~CommandInvoker();

    bool InvokeCommand(QString &strCmd, bool bIsAsync);
    void StopCurCmdExec();
    QString& GetExecutionLog();

private:
    QProcess m_process;
    QString m_logStr;
};

#endif // COMMANDINVOKER_H
