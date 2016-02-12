#ifndef OCHESTRATORPROXY_H
#define OCHESTRATORPROXY_H

#include <QString>
#include <QVariant>

struct ProcessingRequest
{
    int processorId;
    QString parametersJson; // or map<string, string>
};

struct JobDefinition
{
    bool isValid;
    int processorId;
    QString jobDefinitionJson;
};

class OchestratorProxy
{
public:
    OchestratorProxy();
    virtual ~OchestratorProxy();
    virtual JobDefinition GetJobDefinition(const ProcessingRequest &request);
    virtual void SubmitJob(const JobDefinition &job);
};

#endif // OCHESTRATORPROXY_H
