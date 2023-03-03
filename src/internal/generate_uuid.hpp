#pragma once
#include <uuid.h>

namespace reg::internal {

auto generate_uuid() -> uuids::uuid;

} // namespace reg::internal