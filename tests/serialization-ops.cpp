#include "serialization-ops.hpp"

void compare(const ConfigurationParameterInfo &v1, const ConfigurationParameterInfo &v2)
{
    compare(v1.key, v2.key);
    compare(v1.categoryId, v2.categoryId);
    compare(v1.friendlyName, v2.friendlyName);
    compare(v1.dataType, v2.dataType);
    compare(v1.isAdvanced, v2.isAdvanced);
}

void compare(const ConfigurationParameterValue &v1, const ConfigurationParameterValue &v2)
{
    compare(v1.key, v2.key);
    compare(v1.siteId, v2.siteId);
    compare(v1.value, v2.value);
}

void compare(const JobConfigurationParameterValue &v1, const JobConfigurationParameterValue &v2)
{
    compare(v1.key, v2.key);
    compare(v1.value, v2.value);
}

void compare(const ConfigurationCategory &v1, const ConfigurationCategory &v2)
{
    compare(v1.categoryId, v2.categoryId);
    compare(v1.name, v2.name);
    compare(v1.allowPerSiteCustomization, v2.allowPerSiteCustomization);
}

void compare(const Site &v1, const Site &v2)
{
    compare(v1.siteId, v2.siteId);
    compare(v1.name, v2.name);
}

void compare(const ConfigurationSet &v1, const ConfigurationSet &v2)
{
    compare(v1.categories, v2.categories);
    compare(v1.parameterInfo, v2.parameterInfo);
    compare(v1.parameterValues, v2.parameterValues);
    compare(v1.sites, v2.sites);
    compare(v1.isAdmin, v2.isAdmin);
}

void compare(const ConfigurationUpdateAction &v1, const ConfigurationUpdateAction &v2)
{
    compare(v1.key, v2.key);
    compare(v1.siteId, v2.siteId);
    compare(v1.value, v2.value);
}

void compare(const JobConfigurationUpdateAction &v1, const JobConfigurationUpdateAction &v2)
{
    compare(v1.key, v2.key);
    compare(v1.value, v2.value);
}

void compare(const KeyedMessage &v1, const KeyedMessage &v2)
{
    compare(v1.key, v2.key);
    compare(v1.text, v2.text);
}

void compare(const Product &v1, const Product &v2)
{
    compare(v1.productId, v2.productId);
    compare(v1.processorId, v2.processorId);
    compare(v1.productTypeId, v2.productTypeId);
    compare(v1.siteId, v2.siteId);
    compare(v1.fullPath, v2.fullPath);
    compare(v1.created, v2.created);
}

void compare(const ProductToArchive &v1, const ProductToArchive &v2)
{
    compare(v1.productId, v2.productId);
    compare(v1.currentPath, v2.currentPath);
    compare(v1.archivePath, v2.archivePath);
}

void compare(const ArchivedProduct &v1, const ArchivedProduct &v2)
{
    compare(v1.productId, v2.productId);
    compare(v1.archivePath, v2.archivePath);
}

void compare(const NewJob &v1, const NewJob &v2)
{
    compare(v1.name, v2.name);
    compare(v1.description, v2.description);
    compare(v1.processorId, v2.processorId);
    compare(v1.siteId, v2.siteId);
    compare(v1.startType, v2.startType);
    compare(v1.parametersJson, v2.parametersJson);
    compare(v1.configuration, v2.configuration);
}

void compare(const NewTask &v1, const NewTask &v2)
{
    compare(v1.jobId, v2.jobId);
    compare(v1.module, v2.module);
    compare(v1.parametersJson, v2.parametersJson);
    compare(v1.parentTasks, v2.parentTasks);
}

void compare(const NewStep &v1, const NewStep &v2)
{
    compare(v1.taskId, v2.taskId);
    compare(v1.name, v2.name);
    compare(v1.parametersJson, v2.parametersJson);
}

void compare(const ExecutionStatistics &v1, const ExecutionStatistics &v2)
{
    compare(v1.node, v2.node);
    compare(v1.exitCode, v2.exitCode);
    compare(v1.userCpuMs, v2.userCpuMs);
    compare(v1.systemCpuMs, v2.systemCpuMs);
    compare(v1.durationMs, v2.durationMs);
    compare(v1.maxRssKb, v2.maxRssKb);
    compare(v1.maxVmSizeKb, v2.maxVmSizeKb);
    compare(v1.diskReadBytes, v2.diskReadBytes);
    compare(v1.diskWriteBytes, v2.diskWriteBytes);
    compare(v1.stdOutText, v2.stdOutText);
    compare(v1.stdErrText, v2.stdErrText);
}

void compare(const TaskRunnableEvent &v1, const TaskRunnableEvent &v2)
{
    compare(v1.jobId, v2.jobId);
    compare(v1.taskId, v2.taskId);
}

void compare(const TaskFinishedEvent &v1, const TaskFinishedEvent &v2)
{
    compare(v1.processorId, v2.processorId);
    compare(v1.jobId, v2.jobId);
    compare(v1.taskId, v2.taskId);
    compare(v1.module, v2.module);
}

void compare(const ProductAvailableEvent &v1, const ProductAvailableEvent &v2)
{
    compare(v1.productId, v2.productId);
}

void compare(const JobCancelledEvent &v1, const JobCancelledEvent &v2)
{
    compare(v1.jobId, v2.jobId);
}

void compare(const JobPausedEvent &v1, const JobPausedEvent &v2) { compare(v1.jobId, v2.jobId); }

void compare(const JobResumedEvent &v1, const JobResumedEvent &v2) { compare(v1.jobId, v2.jobId); }

void compare(const JobSubmittedEvent &v1, const JobSubmittedEvent &v2)
{
    compare(v1.jobId, v2.jobId);
    compare(v1.processorId, v2.processorId);
    compare(v1.parametersJson, v2.parametersJson);
}

void compare(const StepFailedEvent &v1, const StepFailedEvent &v2)
{
    compare(v1.jobId, v2.jobId);
    compare(v1.taskId, v2.taskId);
    compare(v1.stepName, v2.stepName);
}

void compare(const UnprocessedEvent &v1, const UnprocessedEvent &v2)
{
    compare(v1.eventId, v2.eventId);
    compare(v1.type, v2.type);
    compare(v1.dataJson, v2.dataJson);
    compare(v1.submittedTime, v2.submittedTime);
    compare(v1.processingStartedTime, v2.processingStartedTime);
}

void compare(const NodeStatistics &v1, const NodeStatistics &v2)
{
    compare(v1.node, v2.node);
    compare(v1.cpuUser, v2.cpuUser);
    compare(v1.cpuSystem, v2.cpuSystem);
    compare(v1.memTotalKb, v2.memTotalKb);
    compare(v1.memUsedKb, v2.memUsedKb);
    compare(v1.swapTotalKb, v2.swapTotalKb);
    compare(v1.swapUsedKb, v2.swapUsedKb);
    compare(v1.loadAvg1, v2.loadAvg1);
    compare(v1.loadAvg5, v2.loadAvg5);
    compare(v1.loadAvg15, v2.loadAvg15);
    compare(v1.diskTotalBytes, v2.diskTotalBytes);
    compare(v1.diskUsedBytes, v2.diskUsedBytes);
}

void compare(const StepArgument &v1, const StepArgument &v2) { compare(v1.value, v2.value); }

void compare(const NewExecutorStep &v1, const NewExecutorStep &v2)
{
    compare(v1.taskId, v2.taskId);
    compare(v1.processorPath, v2.processorPath);
    compare(v1.stepName, v2.stepName);
    compare(v1.arguments, v2.arguments);
}

void compare(const JobStepToRun &v1, const JobStepToRun &v2)
{
    compare(v1.taskId, v2.taskId);
    compare(v1.module, v2.module);
    compare(v1.stepName, v2.stepName);
    compare(v1.parametersJson, v2.parametersJson);
}

void compare(const NewProduct &v1, const NewProduct &v2)
{
    compare(v1.productType, v2.productType);
    compare(v1.processorId, v2.processorId);
    compare(v1.taskId, v2.taskId);
    compare(v1.fullPath, v2.fullPath);
    compare(v1.createdTimestamp, v2.createdTimestamp);
}
