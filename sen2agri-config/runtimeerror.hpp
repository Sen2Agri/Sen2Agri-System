#pragma once

#include <string>

#include <QException>

class RuntimeError : public QException
{
    std::string message;

public:
    RuntimeError(const std::string &message);

    const char *what() const noexcept override;

    void raise() const override;
    QException *clone() const override;
};
