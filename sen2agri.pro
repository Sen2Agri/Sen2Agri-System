TEMPLATE = subdirs

SUBDIRS += sen2agri-common \
    sen2agri-config \
    sen2agri-persistence \
    sen2agri-archiver \
    sen2agri-executor \
    sen2agri-orchestrator \
    sen2agri-http-listener \
    QtWebApp

sen2agri-config.depends = sen2agri-common
sen2agri-persistence.depends = sen2agri-common
sen2agri-archiver.depends = sen2agri-common
sen2agri-executor.depends = sen2agri-common
sen2agri-http-listener.depends = sen2agri-common QtWebApp
sen2agri-orchestrator.depends = sen2agri-common
