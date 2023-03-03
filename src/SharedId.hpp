#pragma once

#include <memory>
#include "AnyId.hpp"
#include "internal/IdDestroyer.hpp"

namespace reg {

/// Wraps an ID in a RAII class that will destroy the corresponding object automatically.
/// It can convert into an `Id<T>` implicitly when necessary.
/// It behaves just like a `std::shared_ptr`.
template<typename T>
class SharedId {
public:
    SharedId() = default;

         operator Id<T>() const { return get(); } // NOLINT(*-explicit-constructor, *-explicit-conversions)
         operator AnyId() const { return get(); } // NOLINT(*-explicit-constructor, *-explicit-conversions)
    auto get() const -> Id<T> { return _id_destroyer ? _id_destroyer->id() : Id<T>{}; }

public:
    /// This function is only meant to be called by the implementation.
    /// You should use `registry.create_shared()` instead.
    static auto internal_constructor(Id<T> const& id, std::function<void(Id<T> const&)> destroy)
    {
        auto ret          = SharedId<T>{};
        ret._id_destroyer = std::make_shared<internal::IdDestroyer<T>>(id, destroy);
        return ret;
    }

private:
    std::shared_ptr<internal::IdDestroyer<T>> _id_destroyer{};
};

} // namespace reg
