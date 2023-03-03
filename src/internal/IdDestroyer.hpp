#pragma once
#include <functional>
#include "../Id.hpp"

namespace reg::internal {

/// Responsible for destroying the id automatically when it goes out of scope.
/// It does so by using the `destroy` function that you have to pass to it (this
/// allows us to handle `Registry` and `OrderedRegistry` polymorphically).
template<typename T>
class IdDestroyer {
public:
    IdDestroyer(Id<T> const& id, std::function<void(Id<T> const&)> destroy)
        : _id{id}
        , _destroy{std::move(destroy)}
    {}
    ~IdDestroyer() { _destroy(_id); }
    IdDestroyer(IdDestroyer const&)                        = delete;
    IdDestroyer(IdDestroyer&&) noexcept                    = delete;
    auto operator=(IdDestroyer const&) -> IdDestroyer&     = delete;
    auto operator=(IdDestroyer&&) noexcept -> IdDestroyer& = delete;

    auto id() const -> Id<T> const& { return _id; }

private:
    Id<T>                             _id;
    std::function<void(Id<T> const&)> _destroy;
};

} // namespace reg::internal