#pragma once
#include <unordered_map>
#include "internal/OrderPreservingMap.hpp"
#include "internal/RawRegistryWrapper.hpp"

namespace reg {

template<typename T>
using Registry = internal::RawRegistryWrapper<T, std::unordered_map<Id<T>, T>>;

template<typename T>
using OrderedRegistry = internal::RawRegistryWrapper<T, internal::OrderPreservingMap<Id<T>, T>>;

} // namespace reg