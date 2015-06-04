#include "logger.h"

#include <QDebug>
#include <stdio.h>

#ifndef _vsnprintf
    #define _vsnprintf vsnprintf
#endif

#define MSG_MAX_SIZE 1024

Logger::Logger()
{

}

Logger::~Logger()
{

}

/*static*/
Logger *Logger::GetInstance()
{
    static Logger instance;
    return &instance;
}

void Logger::info(const char* pszMsg, ...)
{
    va_list args;
    va_start(args, pszMsg);

    vTextOut(MSG_TYPE_INFO, pszMsg, args);
}

void Logger::error(const char* pszMsg, ...)
{
    va_list args;
    va_start(args, pszMsg);

    vTextOut(MSG_TYPE_ERROR, pszMsg, args);
}

void Logger::debug(const char* pszMsg, ...)
{
    va_list args;
    va_start(args, pszMsg);

    vTextOut(MSG_TYPE_DEBUG, pszMsg, args);
}

void Logger::vTextOut(int nMsgType, const char* pszMsg, va_list va)
{
    int nMaxSize = MSG_MAX_SIZE;

    char *buf = new char[nMaxSize + 1];
    _vsnprintf(buf, nMaxSize, pszMsg, va);
    buf[nMaxSize] = 0;
    switch(nMsgType) {
        case MSG_TYPE_INFO:
            qWarning() << buf;
            break;
        case MSG_TYPE_ERROR:
            qCritical() << buf;
            break;
        case MSG_TYPE_DEBUG:
            qDebug() << buf;
            break;
        default:
            qWarning() << buf;
            break;
    }
    delete[] buf;
}
