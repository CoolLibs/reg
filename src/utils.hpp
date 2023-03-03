#pragma once
#include <string>
#include "AnyId.hpp"
#include "Id.hpp"

namespace reg {

inline auto to_string(AnyId const& id) -> std::string
{
    return uuids::to_string(id.underlying_uuid());
}

} // namespace reg
