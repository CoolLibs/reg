#pragma once
#include <cstdint>

namespace odb::internal {

class Uuid {
public:
    using StorageType = uint64_t;

    Uuid();

    auto get() const -> StorageType
    {
        return _uuid;
    }

private:
    StorageType _uuid;
};

} // namespace odb::internal
