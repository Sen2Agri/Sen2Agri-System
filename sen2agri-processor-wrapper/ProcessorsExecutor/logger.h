#ifndef LOGGER_H
#define LOGGER_H

#include <cstdarg>

class Logger
{
public:
    ~Logger();

    void info(const char* pszMsg, ...); // intialization and comunications messages
    void error(const char* pszMsg, ...); // error messages
    void debug(const char* pszMsg, ...); // more messagess

    static Logger *GetInstance();

private:
    Logger();
    enum {MSG_TYPE_ERROR=1, MSG_TYPE_DEBUG=2, MSG_TYPE_INFO=4};
    void vTextOut(int nMsgType, const char* pszMsg, va_list va);
};

#endif // LOGGER_H
