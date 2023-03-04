#pragma once
#include "../SharedId.hpp"
#include "../UniqueId.hpp"
#include "RawRegistry.hpp"

namespace reg::internal {

/// Wraps a `RawRegistry` and makes sure its address is always the same in memory.
/// It has the whole interface of a Registry and can be configured to use whichever `Map` type you want.
template<typename T, typename Map>
class RawRegistryWrapper {
public:
    /// The type of values stored in this registry.
    using ValueType = T;

    RawRegistryWrapper()                                                 = default;
    ~RawRegistryWrapper()                                                = default;
    RawRegistryWrapper(RawRegistryWrapper&&) noexcept                    = default;
    auto operator=(RawRegistryWrapper&&) noexcept -> RawRegistryWrapper& = default;

    RawRegistryWrapper(RawRegistryWrapper const&)                    = delete; // This class is non-copyable
    auto operator=(RawRegistryWrapper const&) -> RawRegistryWrapper& = delete; // because it is the unique owner of the objects it stores

    /// Thread-safe.
    /// Returns the value of the objet referenced by `id`, or null if the `id` doesn't refer to a an object in this registry.
    [[nodiscard]] auto get(Id<T> const& id) const -> std::optional<T>
    {
        return _wrapped->get(id);
    }

    /// Thread-safe.
    /// Sets the value of the object referenced by `id` to `value`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    auto set(Id<T> const& id, T const& value) -> bool
    {
        return _wrapped->set(id, value);
    }

    /// Thread-safe.
    /// Returns true iff `id` references an object in the registry.
    [[nodiscard]] auto contains(Id<T> const& id) const -> bool
    {
        return _wrapped->contains(id);
    }

    /// Thread-safe.
    /// Applies `callback` to the object referenced by `id`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    auto with_ref(Id<T> const& id, std::function<void(T const&)> const& callback) const -> bool
    {
        return _wrapped->with_ref(id, callback);
    }

    /// Thread-safe.
    /// Applies `callback` to the object referenced by `id`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    auto with_mutable_ref(Id<T> const& id, std::function<void(T&)> const& callback) -> bool
    {
        return _wrapped->with_mutable_ref(id, callback);
    }

    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    /// Only use this if you need to avoid the copy that `get()` would perform and `with_ref()` doesn't fit your needs.
    [[nodiscard]] auto get_ref(Id<T> const& id) const -> T const*
    {
        return _wrapped->get_ref(id);
    }

    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    /// Only use this if you need to avoid the full assignment that `set()` would perform and `with_mutable_ref()` doesn't fit your needs.
    [[nodiscard]] auto get_mutable_ref(Id<T> const& id) -> T*
    {
        return _wrapped->get_mutable_ref(id);
    }

    /// Thread-safe.
    /// Inserts a copy of `value` into the registry.
    /// Returns the id that will then be used to reference the object that has just been created.
    [[nodiscard]] auto create_unique(T const& value) -> UniqueId<T>
    {
        return UniqueId<T>::internal_constructor(_wrapped->create_raw(value), _wrapped);
    }

    /// Thread-safe.
    /// Inserts a copy of `value` into the registry.
    /// Returns the id that will then be used to reference the object that has just been created.
    [[nodiscard]] auto create_shared(T const& value) -> SharedId<T>
    {
        return SharedId<T>::internal_constructor(_wrapped->create_raw(value), _wrapped);
    }

    /// Thread-safe.
    /// Inserts a copy of `value` into the registry.
    /// Returns the id that will then be used to reference the object that has just been created.
    [[nodiscard]] auto create_raw(T const& value) -> Id<T>
    {
        return _wrapped->create_raw(value);
    }

    /// Thread-safe.
    /// Destroys the object and removes it from the registry.
    /// From then on, trying to get an object using `id` is still safe but will return null.
    void destroy(Id<T> const& id)
    {
        _wrapped->destroy(id);
    }

    /// Thread-safe.
    /// Returns true iff the registry contains no objects at all.
    [[nodiscard]] auto is_empty() const -> bool
    {
        return _wrapped->is_empty();
    }

    /// Thread-safe.
    /// Destroys all the objects in the registry.
    void clear()
    {
        _wrapped->clear();
    }

    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    [[nodiscard]] auto begin() { return _wrapped->begin(); }
    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    [[nodiscard]] auto end() { return _wrapped->end(); }
    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    [[nodiscard]] auto begin() const { return _wrapped->begin(); }
    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    [[nodiscard]] auto end() const { return _wrapped->end(); }
    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    [[nodiscard]] auto cbegin() const { return _wrapped->cbegin(); }
    /// NOT Thread-safe; see the `mutex()` method to make this thread-safe.
    [[nodiscard]] auto cend() const { return _wrapped->cend(); }

    /// Returns the mutex guarding this registry to allow you to lock it manually.
    /// This is only required when using functions that are not already thread-safe: get_ref(), get_mutable_ref(), begin(), end(), cbegin() and cend() (and therefore also using a range-based for loop on this registry).
    /// You should use a std::unique_lock if you want to modify some values, and std::shared_lock if you only need to read them.
    /// See https://stackoverflow.com/a/46050121/15432269 for more details about shared mutexes.
    [[nodiscard]] auto mutex() const -> std::shared_mutex& { return _wrapped->mutex(); }

    [[nodiscard]] auto underlying_container() const -> Map const& { return _wrapped->underlying_container(); }
    [[nodiscard]] auto underlying_container() -> Map& { return _wrapped->underlying_container(); }
    [[nodiscard]] auto underlying_wrapped_registry() -> auto& { return _wrapped; }

private:
    std::shared_ptr<internal::RawRegistry<T, Map>> _wrapped = std::make_shared<internal::RawRegistry<T, Map>>();
};

} // namespace reg::internal