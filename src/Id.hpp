#pragma once
#include <uuid.h>

namespace reg {

template<typename T>
class Id {
public:
    Id() = default;
    /// The type of values referenced by this id.
    using ValueType = T;

    friend auto operator<=>(const Id<T>&, const Id<T>&) = default;

    auto underlying_uuid() -> uuids::uuid& { return _uuid; }
    auto underlying_uuid() const -> const uuids::uuid& { return _uuid; }

private:
    template<typename SomeType>
    friend class Registry;
    friend class AnyId;
    friend std::hash<Id<T>>;

    explicit Id(const uuids::uuid& uuid) // Only a Registry<T> is allowed to create a non-nil ID
        : _uuid{uuid}
    {
    }

    uuids::uuid _uuid{};
};

} // namespace reg

namespace std {
template<typename T>
struct hash<reg::Id<T>> {
    size_t operator()(const reg::Id<T>& id) const
    {
        return std::hash<uuids::uuid>{}(id._uuid);
    }
};
} // namespace std