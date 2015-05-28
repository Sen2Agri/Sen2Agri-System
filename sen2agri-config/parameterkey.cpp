#include "parameterkey.hpp"

#include <utility>

ParameterKey::ParameterKey(QString key, std::experimental::optional<int> siteId)
    : key_(std::move(key)), siteId_(std::move(siteId))
{
}

const QString &ParameterKey::key() const
{
    return key_;
}

std::experimental::optional<int> ParameterKey::siteId() const
{
    return siteId_;
}

bool operator<(const ParameterKey &p1, const ParameterKey &p2)
{
    return p1.key() < p2.key() || (p1.key() == p2.key() && p1.siteId() < p2.siteId());
}
