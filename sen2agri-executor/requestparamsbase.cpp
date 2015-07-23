#include "requestparamsbase.h"


 RequestParamsBase::RequestParamsBase()
 {
     m_ReqType = REQUEST_UNKNOWN;
 }

RequestParamsBase::RequestParamsBase(RequestType reqType)
{
    m_ReqType = reqType;
}

RequestType RequestParamsBase::GetRequestType()
{
    return m_ReqType;
}
