#pragma once
#include <uuid.h>
#include "Id.hpp"

namespace reg {

class AnyId {
public:
    // AnyId() = default; // Not sure if having a default constructor makes sense
    template<typename T>
    AnyId(const Id<T>& id)
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