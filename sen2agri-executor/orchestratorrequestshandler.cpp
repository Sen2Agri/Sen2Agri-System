#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>

#include "orchestratorrequestshandler.h"
#include "ressourcemanageritf.h"
#include "logger.h"

bool OrchestratorRequestsHandler::ExecuteProcessor(const QString &jsonCfgStr)
{
    qDebug() << "Called  ExecuteProcessor with param 1# " << jsonCfgStr;

    QJsonParseError err;
    QByteArray ba(jsonCfgStr.toStdString().c_str());
    QJsonDocument document = QJsonDocument::fromJson(ba, &err);
    if(err.error != QJsonParseError::NoError || !document.isObject())
    {
        Logger::GetInstance()->error("An error occurred during parsing message %s", jsonCfgStr.toStdString().c_str());
        return false;
    }

    QVariantMap msgVals = document.object().toVariantMap();
    msgVals["MSG_TYPE"] = START_PROCESSOR_REQ;
    RessourceManagerItf::GetInstance()->StartProcessor(msgVals);

    return true;
}

bool OrchestratorRequestsHandler::StopProcessorJob(const QString &jobName)
{
    QVariantMap mapVar;

    mapVar["MSG_TYPE"] = STOP_PROCESSOR_REQ;
    mapVar["PROC_JOB_NAME"] = jobName;

    return RessourceManagerItf::GetInstance()->StopProcessor(mapVar);
}

//TODO: Add here also the function:
// GetRunningProcessorJobs
