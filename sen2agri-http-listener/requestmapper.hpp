#pragma once

#include <httpserver/httprequesthandler.h>
#include <httpserver/staticfilecontroller.h>

class RequestMapper : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)

    StaticFileController &staticFileController;

public:
    RequestMapper(StaticFileController &staticFileController, QObject *parent = 0);

    void service(HttpRequest &request, HttpResponse &response);
};
