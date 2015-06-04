#include "orchestratorrequestshandler.h"

#include <QDebug>

#include "QJson/Parser"
#include "ressourcemanageritf.h"

bool OrchestratorRequestsHandler::ExecuteProcessor(const QString &jsonCfgStr)
{
    qDebug() << "Called  ExecuteProcessor with param 1# " << jsonCfgStr;

    QJson::Parser parser;
    bool ok;
    QByteArray ba(jsonCfgStr.toStdString().c_str());

    QVariantMap result = parser.parse (ba, &ok).toMap();
    if (!ok) {
      qFatal("An error occurred during parsing");
      return false;
    }

    RessourceManagerItf::GetInstance()->StartProcessor(result);

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
