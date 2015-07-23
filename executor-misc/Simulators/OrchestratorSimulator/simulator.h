#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "processorsexecutor_interface.h"

class Simulator
{
public:
    Simulator();
    void HandleSendExecuteProcessor();
    void HandleSendCancelProcessor();

private:
    int m_nLastTaskId;
    org::esa::sen2agri::processorsExecutor *m_pProcessorExecutor;
};

#endif // SIMULATOR_H
