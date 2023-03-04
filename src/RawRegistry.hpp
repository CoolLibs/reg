#pragma once
#include <unordered_map>
#include "internal/OrderPreservingMap.hpp"
#include "internal/RawRegistryImpl.hpp"

namespace reg {

/// A `RawRegistry` has all the interface of a Registry
/// except it doesn't have `create_unique()` and `create_shared()`.

template<typename T>
using RawRegistry = internal::RawRegistryImpl<T, std::unordered_map<Id<T>, T>>;

template<typename T>
using RawOrderedRegistry = internal::RawRegistryImpl<T, internal::OrderPreservingMap<Id<T>, T>>;

} // namespace reg