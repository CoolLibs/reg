#pragma once

#include <functional>
#include "Id.hpp"
#include "Registry.hpp"

namespace reg {

// TODO Support all registy types and AnyId
/// This requires that you guarantee that the adress of the `registry` won't change during the lifetime of this `unique_id_owner`. This chould be achieved by allocating the `registry` on the heap (through a `std::unique_ptr` or a `std::shared_ptr`, or by allocating on the stack in a place that will never change, like the beginning of `main`, or by making it a global variable.
/// It is still valid to read the id after the ScopedId has been moved from. It will keep its value. The only difference is that it is no longer responsible for destroying that id when it goes out of scope.
template<typename T>
class ScopedId {
public:
    ScopedId() = default;

    explicit ScopedId(Registry<T>& registry, const T& value = {})
        : _registry{&registry}
        , _id{registry.create(value)}
    {}

    ~ScopedId()
    {
        if (_registry)
        {
            _registry->destroy(_id);
        }
    }

    ScopedId(const ScopedId&)            = delete;
    ScopedId& operator=(const ScopedId&) = delete;

    ScopedId(ScopedId&& rhs) noexcept
        : _registry{rhs._registry}
        , _id{rhs._id}
    {
        rhs._registry = nullptr;
    }

    ScopedId& operator=(ScopedId&& rhs) noexcept
    {
        if (this != &rhs)
        {
            _id           = rhs._id;
            _registry     = rhs._registry;
            rhs._registry = nullptr;
        }

        return *this;
    }

    auto get() const -> Id<T> { return _id; }

    operator Id<T>() const { return get(); }

private:
    Registry<T>* _registry{}; // We use a pointer instead of a reference because we want it to be nullable. This serves both when default-constructing a ScopedId, and also as a bool to know if we have been moved from.
    Id<T>        _id{};
};

} // namespace reg