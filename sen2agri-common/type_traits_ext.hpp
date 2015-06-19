#pragma once

#include <type_traits>

template <typename T>
using is_void_t = typename std::is_void<T>::type;

template <bool b, typename T = void>
using enable_if_t = typename std::enable_if<b, T>::type;
