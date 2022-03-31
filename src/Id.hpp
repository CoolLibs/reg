#pragma once
#include <optional>
#include "Uuid.hpp"

namespace odb {

template<typename T>
class Id {
public:
    Id()            = default;
    using ValueType = T;

private:
    template<typename ThisIdType>
    friend class Database;

    Id(internal::Uuid uuid)
        : _uuid{uuid}
    {
    }

    auto get_uuid()
    {
        return _uuid;
    }

private:
    std::optional<internal::Uuid> _uuid{std::nullopt};
};

} // namespace odb
