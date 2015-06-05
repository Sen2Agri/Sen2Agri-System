#ifndef IPROCESSORWRAPPERMSGSLISTENER_H
#define IPROCESSORWRAPPERMSGSLISTENER_H

#include <QVariantMap>

class IProcessorWrapperMsgsListener
{
public:
    IProcessorWrapperMsgsListener() {}
    virtual ~IProcessorWrapperMsgsListener(){}

    virtual void OnProcessorNewMsg(QVariantMap &msgVals) = 0;
};

#endif // IPROCESSORWRAPPERMSGSLISTENER_H
