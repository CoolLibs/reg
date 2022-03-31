#pragma once

namespace odb::internal {

class Uuid {
public:
    using StorageType = unsigned long long;

    Uuid();

    auto get() const -> StorageType
    {
        return _uuid;
    }

private:
    StorageType _uuid;
};

} // namespace odb::internal
