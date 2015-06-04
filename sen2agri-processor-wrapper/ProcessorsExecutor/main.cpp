#include <stdexcept>

#include <QCoreApplication>

#include <QDBusError>

#include "orchestratorrequestshandler.h"
#include "ProcessorsExecutorAdapter.h"
#include "ressourcemanageritf.h"
#include "configurationmgr.h"
#include "execinfosprotsrvfactory.h"

#define SERVICE_NAME "org.esa.sen2agri.processorsexecutor"

int main(int argc, char *argv[])
{
    QString str;
    QCoreApplication a(argc, argv);

    // Initialize the configuration manager
    str = QString("../../config/ProcessorsExecutorCfg.ini");
    ConfigurationMgr::Initialize(str);

    // Initialize the server for receiving the messages from the executing Processor Wrappers
    QString strIpVal;
    QString strPortVal;
    AbstractExecInfosProtSrv *pExecInfosSrv =
            ExecInfosProtSrvFactory::GetInstance()->CreateExecInfosProtSrv(ExecInfosProtSrvFactory::SIMPLE_UDP);
    str = QString("PROT_SRV_IP_ADDR");
    ConfigurationMgr::GetInstance()->GetValue(str, strIpVal);
    str = QString("PROT_SRV_PORT");
    ConfigurationMgr::GetInstance()->GetValue(str, strPortVal);
    pExecInfosSrv->SetProcMsgListener(RessourceManagerItf::GetInstance());
    pExecInfosSrv->StartCommunication(strIpVal, strPortVal.toInt());

    // start the ressource manager
    RessourceManagerItf::GetInstance()->Start();

    // Initialize the DBus for Orchestrator requests handling
    OrchestratorRequestsHandler orchestratorReqHandler;
    new ProcessorsExecutorAdaptor(&orchestratorReqHandler);

    QDBusConnection connection = QDBusConnection::sessionBus();

    if (!connection.registerObject("/org/esa/sen2agri/processorsexecutor", &orchestratorReqHandler)) {
        QString str = QString("Error registering the object with D-Bus: %1, exiting.")
                .arg(connection.lastError().message());

        throw std::runtime_error(str.toStdString());
    }

    if (!connection.registerService("org.esa.sen2agri.processorsexecutor")) {
        QString str = QString("Error registering the object with D-Bus: %1, exiting.")
                .arg(connection.lastError().message());

        throw std::runtime_error(str.toStdString());
    }

    return a.exec();
}
