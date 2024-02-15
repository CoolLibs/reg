#pragma once
#include <uuid.h>

namespace reg {

namespace internal {
template<typename SomeType, typename Map>
class RawRegistryImpl;
}

template<typename T>
class Id {
public:
    Id() = default;
    explicit Id(uuids::uuid const& uuid)
        : _uuid{uuid}
    {}
    /// The type of values referenced by this id.
    using ValueType = T;

    friend auto operator<=>(const Id<T>&, const Id<T>&) = default;

    [[nodiscard]] auto underlying_uuid() -> uuids::uuid& { return _uuid; }
    [[nodiscard]] auto underlying_uuid() const -> uuids::uuid const& { return _uuid; }

private:
    template<typename SomeType, typename Map>
    friend class internal::RawRegistryImpl;
    friend class AnyId;
    friend std::hash<Id<T>>;

private:
    uuids::uuid _uuid{};
};

} // namespace reg

namespace std {
template<typename T>
struct hash<reg::Id<T>> { // NOLINT(cert-dcl58-cpp)
    auto operator()(const reg::Id<T>& id) const -> size_t
    {
        return std::hash<uuids::uuid>{}(id._uuid);
    }
};
} // namespace std