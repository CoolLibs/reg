#pragma once
#include <unordered_map>
#include "internal/OrderPreservingMap.hpp"
#include "internal/RegistryImpl.hpp"

namespace reg {

template<typename T>
using Registry = internal::RegistryImpl<T, std::unordered_map<Id<T>, T>>;

template<typename T>
using OrderedRegistry = internal::RegistryImpl<T, internal::OrderPreservingMap<Id<T>, T>>;

} // namespace reg