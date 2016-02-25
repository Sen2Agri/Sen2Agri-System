#include "runtimeerror.hpp"

RuntimeError::RuntimeError(const std::string &message) : message(message)
{
}

const char *RuntimeError::what() const noexcept
{
    return message.c_str();
}

void RuntimeError::raise() const
{
    throw * this;
}

QException *RuntimeError::clone() const
{
    return new RuntimeError(message);
}
