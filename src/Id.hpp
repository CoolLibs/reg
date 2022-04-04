#pragma once
#include <uuid.h>

namespace reg {

template<typename T>
class Id {
public:
    Id()            = default;
    using ValueType = T;

    friend auto operator<=>(const Id<T>&, const Id<T>&) = default;

private:
    template<typename SomeType>
    friend class Registry;
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