#pragma once
#include <uuid.h>
#include "Id.hpp"

namespace reg {

class AnyId {
public:
    AnyId() = default;
    AnyId(uuids::uuid const& uuid)
        : _uuid{uuid}
    {}
    template<typename T>
    AnyId(Id<T> const& id)
        : _uuid{id._uuid}
    {}

    friend bool operator==(const AnyId&, const AnyId&) = default;

    auto underlying_uuid() -> uuids::uuid& { return _uuid; }
    auto underlying_uuid() const -> const uuids::uuid& { return _uuid; }

private:
    friend std::hash<AnyId>;

    uuids::uuid _uuid{};
};

} // namespace reg

namespace std {
template<>
struct hash<reg::AnyId> {
    size_t operator()(const reg::AnyId& id) const
    {
        return std::hash<uuids::uuid>{}(id._uuid);
    }
};
} // namespace std