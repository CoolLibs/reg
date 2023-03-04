#pragma once

#include <memory>
#include "AnyId.hpp"
#include "internal/IdDestroyer.hpp"

namespace reg {

/// Wraps an ID in a RAII class that will destroy the corresponding object automatically.
/// It can convert into an `Id<T>` implicitly when necessary.
/// It behaves just like a `std::unique_ptr`.
template<typename T>
class UniqueId {
public:
    UniqueId() = default;

    auto raw() const -> Id<T> { return _id_destroyer ? _id_destroyer->id() : Id<T>{}; }

public:
    /// This function is only meant to be called by the implementation.
    /// You should use `registry.create_unique()` instead.
    static auto internal_constructor(Id<T> const& id, internal::AnyRawRegistry<T> registry)
    {
        auto ret          = UniqueId<T>{};
        ret._id_destroyer = std::make_unique<internal::IdDestroyer<T>>(id, registry);
        return ret;
    }

    auto underlying_object() -> auto& { return _id_destroyer; }

private:
    std::unique_ptr<internal::IdDestroyer<T>> _id_destroyer{};
};

} // namespace reg
