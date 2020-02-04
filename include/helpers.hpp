#pragma once

#include <array>

// This just provides a way of declaring a std::array
// with some elements and not having to specify the size explicitly.
//
// From: https://stackoverflow.com/a/6114359/840973
template<typename ret, typename... T>
std::array<ret, sizeof...(T)> make_array(T&&... refs) {
  return { std::forward<T>(refs)... };
};
