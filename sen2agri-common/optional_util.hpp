#pragma once

template <typename T>
std::experimental::optional<T> to_optional(const QVariant &v)
{
    if (v.isNull()) {
        return std::experimental::nullopt;
    }
    return v.value<T>();
}
