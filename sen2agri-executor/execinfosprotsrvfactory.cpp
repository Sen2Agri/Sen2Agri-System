#include "execinfosprotsrvfactory.h"
#include "abstractexecinfosprotsrv.h"
#include "simpleudpexecinfosprotsrv.h"

ExecInfosProtSrvFactory::ExecInfosProtSrvFactory()
{
    Register(SIMPLE_UDP, &SimpleUdpExecInfosProtSrv::Create);
    //TODO
    //Register(HTTP_SRV, &HttpExecInfosProtSrv::Create);
}

ExecInfosProtSrvFactory::~ExecInfosProtSrvFactory()
{
    m_FactoryMap.clear();
}

void ExecInfosProtSrvFactory::Register(const int srvType, CreateExecInfosProtSrvFn pfnCreate)
{
    m_FactoryMap[srvType] = pfnCreate;
}

/*static*/
ExecInfosProtSrvFactory *ExecInfosProtSrvFactory::GetInstance()
{
    static ExecInfosProtSrvFactory instance;
    return &instance;
}

AbstractExecInfosProtSrv *ExecInfosProtSrvFactory::CreateExecInfosProtSrv(const int srvType)
{
    FactoryMap::iterator it = m_FactoryMap.find(srvType);
    if( it != m_FactoryMap.end() )
        return it->second();
    return NULL;
}
