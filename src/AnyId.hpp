#pragma once
#include <uuid.h>
#include "Id.hpp"

namespace reg {

class AnyId {
public:
    AnyId() = default;
    AnyId(uuids::uuid const& uuid) // NOLINT(*-explicit-constructor, *-explicit-conversions)
        : _uuid{uuid}
    {}
    template<typename T>
    AnyId(Id<T> const& id) // NOLINT(*-explicit-constructor, *-explicit-conversions)
        : _uuid{id._uuid}
    {}
    template<typename T>
    AnyId(Id<T>&& id) noexcept // NOLINT(*-explicit-constructor, *-explicit-conversions)
        : _uuid{std::move(id._uuid)}
    {}
    template<typename T>
    auto operator=(Id<T> const& id) -> AnyId&
    {
        _uuid = id._uuid;
        return *this;
    }
    template<typename T>
    auto operator=(Id<T>&& id) noexcept -> AnyId&
    {
        _uuid = std::move(id._uuid);
        return *this;
    }

    template<typename T>
    operator Id<T>() const // NOLINT(*-explicit-constructor, *-explicit-conversions)
    {
        return Id<T>{_uuid};
    }

    friend auto operator==(AnyId const&, AnyId const&) -> bool = default;

    template<typename T>
    friend auto operator==(AnyId const& id1, Id<T> const& id2) -> bool
    {
        return id1 == AnyId{id2};
    }
    template<typename T>
    friend auto operator==(Id<T> const& id1, AnyId const& id2) -> bool
    {
        return id2 == id1;
    }

    [[nodiscard]] auto underlying_uuid() -> uuids::uuid& { return _uuid; }
    [[nodiscard]] auto underlying_uuid() const -> uuids::uuid const& { return _uuid; }

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