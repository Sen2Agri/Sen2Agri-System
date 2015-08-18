#include "requestparamsexecutioninfos.h"

RequestParamsExecutionInfos::RequestParamsExecutionInfos()
    : RequestParamsBase(PROCESSOR_EXECUTION_INFO_MSG)
{

}

bool RequestParamsExecutionInfos::ParseMessage(const QByteArray &message)
{
    QJsonParseError err;
    QJsonDocument document = QJsonDocument::fromJson(message, &err);
    if(err.error != QJsonParseError::NoError || !document.isObject()) {
        return false;
    }

    m_msgVals = document.object().toVariantMap();
    if (m_msgVals.contains("JOB_NAME"))
        m_strJobName = m_msgVals.value("JOB_NAME").toString();
    else
        m_strJobName = "";

    if (m_msgVals.contains("MSG_TYPE"))
        m_strMsgType = m_msgVals.value("MSG_TYPE").toString();
    else
        m_strMsgType = "";
    return true;
}

bool RequestParamsExecutionInfos::IsExecutionStarted()
{
    if(m_strMsgType == "STARTED")
        return true;
    return false;
}


bool RequestParamsExecutionInfos::IsExecutionEnded()
{
    if(m_strMsgType == "ENDED")
        return true;
    return false;
}

bool RequestParamsExecutionInfos::IsLogMsg()
{
    if(m_strMsgType == "LOG")
        return true;
    return false;
}

const QString& RequestParamsExecutionInfos::GetJobName()
{
    return m_strJobName;
}

const QString RequestParamsExecutionInfos::GetLogMsg()
{
    if (m_msgVals.contains("LOG_MSG"))
        return m_msgVals.value("LOG_MSG").toString();
    return "";
}

const QString RequestParamsExecutionInfos::GetExecutionTime()
{
    if (m_msgVals.contains("EXEC_TIME"))
        return m_msgVals.value("EXEC_TIME").toString();
    return "";
}


