#pragma once

#include <stdexcept>

#include <QSqlError>

class sql_error : public std::runtime_error
{
    std::string error_code_;

public:
    sql_error(const QSqlError &error);

    const std::string &error_code() const;
};
