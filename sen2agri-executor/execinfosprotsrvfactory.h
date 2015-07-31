#ifndef EXECINFOSPROTOCOLSRVFACTORY_H
#define EXECINFOSPROTOCOLSRVFACTORY_H

#include <map>
#include <string>
using namespace std;

#include "abstractexecinfosprotsrv.h"

class ExecInfosProtSrvFactory
{
public:
    ~ExecInfosProtSrvFactory();

    enum {SIMPLE_UDP, HTTP_SRV};

private:
    ExecInfosProtSrvFactory();
    ExecInfosProtSrvFactory(const ExecInfosProtSrvFactory &) { }
    ExecInfosProtSrvFactory &operator=(const ExecInfosProtSrvFactory &) { return *this; }

    typedef map<int, CreateExecInfosProtSrvFn> FactoryMap;
    FactoryMap m_FactoryMap;

public:
    static ExecInfosProtSrvFactory *GetInstance();
    void Register(const int srvType, CreateExecInfosProtSrvFn pfnCreate);
    AbstractExecInfosProtSrv *CreateExecInfosProtSrv(const int srvType);
};

#endif // EXECINFOSPROTOCOLSRVFACTORY_H
