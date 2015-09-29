#include <limits>
#include <sstream>

#include <otbMacro.h>

#include "string_utils.hpp"

std::vector<std::string> split(const std::string &s, char delim)
{
    std::vector<std::string> result;

    std::istringstream is(s);
    std::string item;
    while (std::getline(is, item, delim)) {
        result.emplace_back(std::move(item));
    }

    return result;
}

bool ends_with(const std::string &s1, const std::string &s2)
{
    return s1.length() >= s2.length() &&
           s1.compare(s1.length() - s2.length(), s2.length(), s2) == 0;
}

double ReadDouble(const std::string &s)
{
    try {
        if (s.empty()) {
            return std::numeric_limits<double>::quiet_NaN();
        }

        return std::stod(s);
    } catch (const std::exception &e) {
        otbMsgDevMacro("Invalid double value " << s << ": " << e.what());

        return std::numeric_limits<double>::quiet_NaN();
    }
}
