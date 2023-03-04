#pragma once
#include <functional>
#include <variant>
#include "../Id.hpp"
#include "RawRegistry.hpp"

namespace reg::internal {

template<typename T>
using AnyRawRegistry = std::variant<std::weak_ptr<RawUsualRegistry<T>>, std::weak_ptr<RawOrderedRegistry<T>>>;

/// Responsible for destroying the id automatically when it goes out of scope.
/// It does so by using the `destroy` function that you have to pass to it (this
/// allows us to handle `Registry` and `OrderedRegistry` polymorphically).
template<typename T>
class IdDestroyer {
public:
    IdDestroyer() = default; // For serialization
    IdDestroyer(Id<T> const& id, AnyRawRegistry<T> registry)
        : _id{id}
        , _registry{std::move(registry)}
    {}
    ~IdDestroyer()
    {
        std::visit([&](auto&& registry) {
            if (auto shared_ptr = registry.lock())
                shared_ptr->destroy(_id);
        },
                   _registry);
    }
    IdDestroyer(IdDestroyer const&)                        = delete;
    IdDestroyer(IdDestroyer&&) noexcept                    = delete;
    auto operator=(IdDestroyer const&) -> IdDestroyer&     = delete;
    auto operator=(IdDestroyer&&) noexcept -> IdDestroyer& = delete;

    auto id() const -> Id<T> const& { return _id; }

    auto underlying_uuid() -> auto& { return _id.underlying_uuid(); }
    auto underlying_registry() -> auto& { return _registry; }

private:
    Id<T>             _id;
    AnyRawRegistry<T> _registry;
};

} // namespace reg::internal