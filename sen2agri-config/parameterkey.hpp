#pragma once

#include <QString>

#include "optional.hpp"

class ParameterKey
{
    QString key_;
    std::experimental::optional<int> siteId_;

public:
    ParameterKey(QString key, std::experimental::optional<int> siteId);

    const QString &key() const;
    std::experimental::optional<int> siteId() const;
};

bool operator<(const ParameterKey &p1, const ParameterKey &p2);
