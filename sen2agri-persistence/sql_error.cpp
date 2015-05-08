#include "sql_error.hpp"

using std::runtime_error;
using std::string;

sql_error::sql_error(const QSqlError &error)
    : runtime_error(error.text().toStdString()), error_code_(error.nativeErrorCode().toStdString())
{
}

const string &sql_error::error_code() const
{
    return error_code_;
}
