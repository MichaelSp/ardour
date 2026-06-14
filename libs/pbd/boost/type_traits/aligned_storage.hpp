#pragma once

#include <cstddef>
#include <type_traits>

namespace boost {

template <std::size_t Len, std::size_t Align>
struct aligned_storage {
	using type = typename std::aligned_storage<Len, Align>::type;
};

} // namespace boost
