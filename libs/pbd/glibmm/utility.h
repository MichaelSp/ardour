#ifndef ARDOUR_GLIBMM_UTILITY_H
#define ARDOUR_GLIBMM_UTILITY_H

#include_next <glibmm/utility.h>

namespace Glib
{

template <typename T>
constexpr T unconst(const T& value)
{
  return const_cast<T>(value);
}

} // namespace Glib

#endif
