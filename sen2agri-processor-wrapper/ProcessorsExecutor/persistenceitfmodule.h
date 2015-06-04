#ifndef PERSISTENCEITFMODULE_H
#define PERSISTENCEITFMODULE_H


/**
 * @brief The PersistenceItfModule class
 * \note
 * This class represents the interface with the persistence manager
 * application. The communication is made via DBus.
 *
 */

#include "processorexecutioninfos.h"

class PersistenceItfModule
{
public:
    ~PersistenceItfModule();

    static PersistenceItfModule *GetInstance();

    void SendProcessorStart(ProcessorExecutionInfos &execInfos);
    void SendProcessorEnd(ProcessorExecutionInfos &execInfos);
    void SendProcessorCancel(ProcessorExecutionInfos &execInfos);

private:
    PersistenceItfModule();
};

#endif // PERSISTENCEITFMODULE_H
