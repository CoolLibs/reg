#pragma once
#include <cpp-typelist/typelist.hpp>
#include <tuple>
#include "Registry.hpp"

namespace reg {

/// Thanks to https://ngathanasiou.wordpress.com/2020/07/09/avoiding-compile-time-recursion/
namespace internal {

template<class T, std::size_t I, class Tuple>
constexpr bool match_v = std::is_same_v<T, std::tuple_element_t<I, Tuple>>;

template<class T, class Tuple,
         class Idxs = std::make_index_sequence<std::tuple_size_v<Tuple>>>
struct type_index;

template<class T, template<class...> class Tuple, class... Args,
         std::size_t... Is>
struct type_index<T, Tuple<Args...>, std::index_sequence<Is...>>
    : std::integral_constant<std::size_t,
                             ((Is * match_v<T, Is, Tuple<Args...>>)+... + 0)> {
    static_assert(2 > (match_v<T, Is, Tuple<Args...>> + ... + 0),
                  "T was declared multiple types in Registries");
    static_assert(0 != (match_v<T, Is, Tuple<Args...>> + ... + 0),
                  "T was not declared as one of the types of Registries");
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
    auto of() const -> const Registry<T>&
    {
        return std::get<internal::type_index_v<reg::Registry<T>, Tuple>>(_registries);
    }

    template<typename T>
    [[nodiscard]] auto get(const Id<T>& id) const -> std::optional<T>
    {
        return of<T>().get(id);
    }

    template<typename T>
    auto set(const Id<T>& id, const T& value) -> bool
    {
        return of<T>().set(id, value);
    }

    template<typename T>
    [[nodiscard]] auto create(const T& value) -> Id<T>
    {
        return of<T>().create(value);
    }

    template<typename T>
    void destroy(const Id<T>& id)
    {
        of<T>().destroy(id);
    }

    using Tuple = typename tl::type_list<Ts...>::template wrap<reg::Registry>::to_tuple; // Create a tuple of reg::Registry<T> for each T in Ts

    auto underlying_registries() const -> const Tuple& { return _registries; }
    auto underlying_registries() -> Tuple& { return _registries; }

private:
    Tuple _registries{};
};

} // namespace reg