#pragma once

#include <functional>
#include <memory>
#include "Id.hpp"
#include "Registry.hpp"

namespace reg {

namespace internal {

template<typename RegistryT, typename T>
class IdAndReg {
public:
    IdAndReg(Id<T> const& id, RegistryT& registry)
        : _id{id}, _registry{&registry}
    {}
    ~IdAndReg() { _registry->destroy(_id); }

    auto id() const -> Id<T> const& { return _id; }

private:
    Id<T>      _id;
    RegistryT* _registry; // TODO(JF) Use the underlying address of the registry that will never change
};

template<typename RegistryT, typename T>
class ScopedId {
public:
    ScopedId() = default;

    explicit ScopedId(RegistryT& registry, const T& value = {})
        : _id_and_registry{std::make_unique<IdAndReg<RegistryT, T>>(registry.create(value), registry)}
    {}

    auto get() const -> Id<T> { return _id_and_registry ? _id_and_registry->id() : Id<T>{}; }

private:
    std::unique_ptr<IdAndReg<RegistryT, T>> _id_and_registry{};
};

} // namespace internal

/// Wraps an ID in a RAII class that will destroy the corresponding object automatically.
/// This requires that you guarantee that the address of the `registry` won't change during the lifetime of this `ScopedId`
/// because the `ScopedId` stores a reference to that `registry`.
/// This can be achieved:
/// - By allocating the `registry` on the heap (through a `std::unique_ptr` or a `std::shared_ptr`)
/// - By allocating the `registry` on the stack in a parent scope relative to this `ScopedId`, like the beginning of `main`
/// - By making the `registry` a global variable.
/// It is still valid to read the id after the ScopedId has been moved from. It will keep its value. The only difference is that it is no longer responsible for destroying that id when it goes out of scope.
template<typename T>
class ScopedId {
public:
    ScopedId() = default;
    explicit ScopedId(Registry<T>& registry, const T& value = {})
        : _scoped_id{registry, value}
    {}

    auto get() const -> Id<T> { return _scoped_id.get(); }

    operator Id<T>() const { return get(); }

private:
    internal::ScopedId<Registry<T>, T> _scoped_id;
};

/// Wraps an ID in a RAII class that will destroy the corresponding object automatically.
/// This requires that you guarantee that the address of the `registry` won't change during the lifetime of this `ScopedId`
/// because the `ScopedId` stores a reference to that `registry`.
/// This can be achieved:
/// - By allocating the `registry` on the heap (through a `std::unique_ptr` or a `std::shared_ptr`)
/// - By allocating the `registry` on the stack in a parent scope relative to this `ScopedId`, like the beginning of `main`
/// - By making the `registry` a global variable.
/// It is still valid to read the id after the ScopedId has been moved from. It will keep its value. The only difference is that it is no longer responsible for destroying that id when it goes out of scope.
template<typename T>
class ScopedId_Ordered {
public:
    ScopedId_Ordered() = default;
    explicit ScopedId_Ordered(OrderedRegistry<T>& registry, const T& value = {})
        : _scoped_id{registry, value}
    {}

    auto get() const -> Id<T> { return _scoped_id.get(); }

    operator Id<T>() const { return get(); }

private:
    internal::ScopedId<OrderedRegistry<T>, T> _scoped_id;
};

} // namespace reg
