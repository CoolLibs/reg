#pragma once
#include <uuid.h>

namespace odb {

template<typename T>
class Id {
public:
    Id()            = default;
    using ValueType = T;

private:
    template<typename ThisIdType>
    friend class Database;

    Id(uuids::uuid uuid)
        : _uuid{uuid}
    {
    }

    auto get_uuid()
    {
        return _uuid;
    }

private:
    uuids::uuid _uuid{};
};

} // namespace odb
