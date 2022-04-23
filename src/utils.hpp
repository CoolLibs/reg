#pragma once
#include <string>
#include "AnyId.hpp"
#include "Id.hpp"

namespace reg {

inline auto to_string(const AnyId& id) -> std::string
{
    return uuids::to_string(id.underlying_uuid());
}

template<typename T>
auto to_string(const Id<T>& id) -> std::string
{
    return uuids::to_string(id.underlying_uuid());
}

} // namespace reg
