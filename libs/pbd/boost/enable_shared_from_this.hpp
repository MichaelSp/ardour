#pragma once

#include <memory>

namespace boost {
template<typename T>
using enable_shared_from_this = std::enable_shared_from_this<T>;
}
