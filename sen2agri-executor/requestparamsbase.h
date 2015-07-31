#ifndef REQUESTPARAMSBASE_H
#define REQUESTPARAMSBASE_H


typedef enum RequestType
{
    PROCESSOR_EXECUTION_INFO_MSG = 1,
    START_PROCESSOR_REQ = 2,
    STOP_PROCESSOR_REQ = 3,
    REQUEST_UNKNOWN = 256
} RequestType;


class RequestParamsBase
{
public:
    RequestParamsBase();
    RequestParamsBase(RequestType reqType);
    RequestType GetRequestType();

protected:
    RequestType m_ReqType;
};

#endif // REQUESTPARAMSBASE_H
