#include "testorcherstratorproxy.h"

TestOrcherstratorProxy::TestOrcherstratorProxy() :
    m_validJobs(true)
{

}

TestOrcherstratorProxy::~TestOrcherstratorProxy()
{
}

// app API
JobDefinition TestOrcherstratorProxy::GetJobDefinition(const ProcessingRequest &request)
{
    JobDefinition jd;
    jd.isValid = m_validJobs;
    jd.processorId = request.processorId;
    jd.jobDefinitionJson = request.parametersJson;

    return jd;
}

void TestOrcherstratorProxy::SubmitJob(const JobDefinition &job)
{
    m_submitedJobs.push_back(job);
}

void TestOrcherstratorProxy::setNextJobsValid(bool bValid)
{
    m_validJobs = bValid;
}

std::vector<JobDefinition> TestOrcherstratorProxy::getSubmitedJobs()
{
    return m_submitedJobs;
}

void TestOrcherstratorProxy::resetSubmitedJobs()
{
    m_submitedJobs.clear();
}
