#pragma once

#include <httpserver/httprequesthandler.h>
#include <httpserver/staticfilecontroller.h>

#include "persistencemanager.hpp"

class RequestMapper : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)

    StaticFileController &staticFileController;
    PersistenceManagerDBProvider &persistenceManager;

public:
    RequestMapper(StaticFileController &staticFileController,
                  PersistenceManagerDBProvider &persistenceManager,
                  QObject *parent = 0);

    void service(HttpRequest &request, HttpResponse &response);
};
