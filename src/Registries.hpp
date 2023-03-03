#pragma once
#include <tuple>
#include "Registry.hpp"

namespace reg {

/// Thanks to https://ngathanasiou.wordpress.com/2020/07/09/avoiding-compile-time-recursion/
namespace internal {

template<class T, std::size_t I, class Tuple>
constexpr bool match_v = std::is_same_v<T, std::tuple_element_t<I, Tuple>>;

template<class T, class Tuple, class Idxs = std::make_index_sequence<std::tuple_size_v<Tuple>>>
struct type_index;

template<class T, template<class...> class Tuple, class... Args, std::size_t... Is>
struct type_index<T, Tuple<Args...>, std::index_sequence<Is...>>
    : std::integral_constant<std::size_t, ((Is * match_v<T, Is, Tuple<Args...>>)+... + 0)> {
    static_assert(2 > (match_v<T, Is, Tuple<Args...>> + ... + 0), "T was declared multiple times in Registries");
    static_assert(0 != (match_v<T, Is, Tuple<Args...>> + ... + 0), "T was not declared as one of the types of Registries");
};

template<class T, class Tuple>
constexpr std::size_t type_index_v = type_index<T, Tuple>::value;

} // namespace internal

template<typename... Ts>
class Registries {
public:
    template<typename T>
    auto of() -> Registry<T>&
    {
        return std::get<internal::type_index_v<reg::Registry<T>, Tuple>>(_registries);
    }

    template<typename T>
    auto of() const -> Registry<T> const&
    {
        return std::get<internal::type_index_v<reg::Registry<T>, Tuple>>(_registries);
    }

    /// Thread-safe.
    /// Returns the value of the objet referenced by `id`, or null if the `id` doesn't refer to a an object in this registry.
    template<typename T>
    [[nodiscard]] auto get(Id<T> const& id) const -> std::optional<T>
    {
        return of<T>().get(id);
    }

    /// Thread-safe.
    /// Sets the value of the object referenced by `id` to `value`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    template<typename T>
    auto set(Id<T> const& id, T const& value) -> bool
    {
        return of<T>().set(id, value);
    }

    /// Thread-safe.
    /// Applies `callback` to the object referenced by `id`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    template<typename T>
    auto with_ref(Id<T> const& id, std::function<void(T const&)> const& callback) const -> bool
    {
        return of<T>().with_ref(id, callback);
    }

    /// Thread-safe.
    /// Applies `callback` to the object referenced by `id`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    template<typename T>
    auto with_mutable_ref(Id<T> const& id, std::function<void(T&)> const& callback) -> bool
    {
        return of<T>().with_mutable_ref(id, callback);
    }

    /// Thread-safe.
    /// Inserts a copy of `value` into the registry.
    /// Returns the id that will then be used to reference the object that has just been created.
    template<typename T>
    [[nodiscard]] auto create_unique(T const& value) -> UniqueId<T>
    {
        return of<T>().create_unique(value);
    }

    /// Thread-safe.
    /// Inserts a copy of `value` into the registry.
    /// Returns the id that will then be used to reference the object that has just been created.
    template<typename T>
    [[nodiscard]] auto create_shared(T const& value) -> SharedId<T>
    {
        return of<T>().create_shared(value);
    }

    /// Thread-safe.
    /// Returns true iff the registry contains no objects at all.
    template<typename T>
    [[nodiscard]] auto is_empty() const -> bool
    {
        return of<T>().is_empty();
    }

    /// Thread-safe.
    /// Destroys all the objects in the registry.
    template<typename T>
    void clear()
    {
        of<T>().clear();
    }

    /// Returns the mutex guarding this registry to allow you to lock it manually.
    /// This is only required when using functions that are not already thread-safe: get_ref(), get_mutable_ref(), begin(), end(), cbegin() and cend() (and therefore also using a range-based for loop on this registry).
    /// You should use a std::unique_lock if you want to modify some values, and std::shared_lock if you only need to read them.
    /// See https://stackoverflow.com/a/46050121/15432269 for more details about shared mutexes.
    [[nodiscard]] auto mutex() const -> std::shared_mutex& { return of<T>().mutex(); }

    using Tuple = std::tuple<Ts...>;

    [[nodiscard]] auto underlying_registries() const -> Tuple const& { return _registries; }
    [[nodiscard]] auto underlying_registries() -> Tuple& { return _registries; }

private:
    Tuple _registries{};
};

} // namespace reg
