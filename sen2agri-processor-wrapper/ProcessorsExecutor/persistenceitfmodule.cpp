#include "persistenceitfmodule.h"

#include <iostream>
using namespace std;

PersistenceItfModule::PersistenceItfModule()
{

}

PersistenceItfModule::~PersistenceItfModule()
{

}

/*static*/
PersistenceItfModule *PersistenceItfModule::GetInstance()
{
    static PersistenceItfModule instance;
    return &instance;
}

void PersistenceItfModule::SendProcessorStart(ProcessorExecutionInfos &execInfos)
{
    cout << "---------------------------------------------------" << endl;
    cout << "PersistenceItfModule : SendProcessorStart called !!!" << endl;
    cout << "---------------------------------------------------" << endl;
    // TODO: Send via DBus
}

void PersistenceItfModule::SendProcessorEnd(ProcessorExecutionInfos &execInfos)
{
    cout << "---------------------------------------------------" << endl;
    cout << "PersistenceItfModule : SendProcessorEnd called !!!" << endl;
    cout << "---------------------------------------------------" << endl;
    // TODO: Send via DBus
}

void PersistenceItfModule::SendProcessorCancel(ProcessorExecutionInfos &execInfos)
{
    cout << "---------------------------------------------------" << endl;
    cout << "PersistenceItfModule : SendProcessorCancel called !!!" << endl;
    cout << "---------------------------------------------------" << endl;
    // TODO: Send via DBus
}
