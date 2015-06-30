#pragma once

#include <httpserver/httprequesthandler.h>

class RequestMapper : public HttpRequestHandler
{
    Q_OBJECT
    Q_DISABLE_COPY(RequestMapper)

public:
    RequestMapper(QObject *parent = 0);

    void service(HttpRequest &request, HttpResponse &response);
};
