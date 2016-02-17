TEMPLATE = subdirs

SUBDIRS += sen2agri-common \
    sen2agri-config \
    sen2agri-persistence \
    sen2agri-archiver \
    sen2agri-executor \
    sen2agri-orchestrator \
    sen2agri-http-listener \
    sen2agri-monitor-agent \
    sen2agri-processor-wrapper \
    sen2agri-scheduler \
    QtWebApp \
    tests

sen2agri-config.depends = sen2agri-common sen2agri-persistence
sen2agri-persistence.depends = sen2agri-common
sen2agri-archiver.depends = sen2agri-common sen2agri-persistence
sen2agri-executor.depends = sen2agri-common sen2agri-persistence
sen2agri-orchestrator.depends = sen2agri-common sen2agri-persistence
sen2agri-http-listener.depends = sen2agri-common sen2agri-persistence QtWebApp
sen2agri-monitor-agent.depends = sen2agri-common
sen2agri-scheduler.depends = sen2agri-common sen2agri-persistence
tests.depends = sen2agri-common sen2agri-scheduler sen2agri-persistence
