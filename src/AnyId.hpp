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
    {
    }

    friend bool operator==(const AnyId&, const AnyId&) = default;
    // template<typename T>
    // friend bool operator==(const AnyId& id1, const Id<T>& id2)
    // {
    //     return id1._uuid == id2._uuid;
    // }
    // template<typename T>
    // friend bool operator==(const Id<T>& id2, const AnyId& id1)
    // {
    //     return id1 == id2;
    // }
    // template<typename T>
    // friend bool operator!=(const AnyId& id1, const Id<T>& id2)
    // {
    //     return !(id1 == id2);
    // }
    // template<typename T>
    // friend bool operator!=(const Id<T>& id2, const AnyId& id1)
    // {
    //     return !(id1 == id2);
    // }

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