#ifndef TESTORCHERSTRATORPROXY_H
#define TESTORCHERSTRATORPROXY_H

#include "ochestratorproxy.hpp"

class TestOrcherstratorProxy : public OchestratorProxy
{
public:
    TestOrcherstratorProxy();
    virtual ~TestOrcherstratorProxy();

    // app API
    virtual JobDefinition GetJobDefinition(const ProcessingRequest &request);
    virtual void SubmitJob(const JobDefinition &job);

    // test API
    void setNextJobsValid(bool bValid);
    std::vector<JobDefinition> getSubmitedJobs();
    void resetSubmitedJobs();

private:
    std::vector<JobDefinition> m_submitedJobs;
    bool m_validJobs;
};

#endif // TESTORCHERSTRATORPROXY_H
