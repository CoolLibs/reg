#pragma once
#include <uuid.h>
#include <array>
#include <optional>
#include <random>
#include <shared_mutex>
#include <unordered_map>
#include "Id.hpp"

namespace reg {

namespace internal {

inline auto create_random_generator()
{
    std::random_device rd;
    auto               seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    std::seed_seq seq(std::begin(seed_data), std::end(seed_data));
    return std::mt19937{seq};
}

inline auto generate_uuid() -> uuids::uuid
{
    static thread_local auto generator{create_random_generator()};
    return uuids::uuid_random_generator{generator}();
}

} // namespace internal

template<typename T>
class Registry {
public:
    /// The type of values stored in this registry.
    using ValueType = T;

    Registry()                               = default;
    ~Registry()                              = default;
    Registry(Registry&&) noexcept            = default;
    Registry& operator=(Registry&&) noexcept = default;

    Registry(const Registry&)            = delete; // This class is non-copyable
    Registry& operator=(const Registry&) = delete; // because it is the unique owner of the objects it stores

    /// Thread-safe.
    /// Returns the value of the objet referenced by `id`, or null if the `id` doesn't refer to a an object in this registry.
    [[nodiscard]] auto get(const Id<T>& id) const -> std::optional<T>
    {
        std::shared_lock lock{_mutex};

        const auto it = _map.find(id);
        if (it == _map.end())
        {
            return std::nullopt;
        }
        else
        {
            return it->second;
        }
    }

    /// Thread-safe.
    /// Sets the value of the object referenced by `id` to `value`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    auto set(const Id<T>& id, const T& value) -> bool
    {
        std::unique_lock lock{_mutex};

        auto it = _map.find(id);
        if (it != _map.end())
        {
            it->second = value;
            return true;
        }
        else
        {
            return false;
        }
    }

    /// Thread-safe.
    /// Returns iff `id` references an object in the registry.
    [[nodiscard]] auto contains(const Id<T>& id) const -> bool
    {
        std::shared_lock lock{_mutex};

        const auto it = _map.find(id);
        return it != _map.end();
    }

    /// Thread-safe.
    /// Applies `callback` to the object referenced by `id`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    auto with_ref(const Id<T>& id, std::function<void(const T&)> callback) const -> bool
    {
        std::shared_lock lock{_mutex};

        const auto it = _map.find(id);
        if (it != _map.end())
        {
            callback(it->second);
            return true;
        }
        else
        {
            return false;
        }
    }

    /// Thread-safe.
    /// Applies `callback` to the object referenced by `id`.
    /// Does nothing if the `id` doesn't refer to an object in this registry.
    /// Returns false iff the object was not found in the registry and this function did nothing.
    auto with_mutable_ref(const Id<T>& id, std::function<void(T&)> callback) -> bool
    {
        std::unique_lock lock{_mutex};

        const auto it = _map.find(id);
        if (it != _map.end())
        {
            callback(it->second);
            return true;
        }
        else
        {
            return false;
        }
    }

    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    /// Only use this if you need to avoid the copy that get() would perform.
    [[nodiscard]] auto get_ref(const Id<T>& id) const -> const T*
    {
        const auto it = _map.find(id);
        if (it == _map.end())
        {
            return nullptr;
        }
        else
        {
            return &it->second;
        }
    }

    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    /// Only use this if you need to avoid the full assignment that set() would perform.
    [[nodiscard]] auto get_mutable_ref(const Id<T>& id) -> T*
    {
        auto it = _map.find(id);
        if (it == _map.end())
        {
            return nullptr;
        }
        else
        {
            return &it->second;
        }
    }

    /// Thread-safe.
    /// Inserts a copy of `value` into the registry.
    /// Returns the id that will then be used to reference the object that has just been created.
    [[nodiscard]] auto create(const T& value) -> Id<T>
    {
        const auto id = Id<T>{internal::generate_uuid()};

        std::unique_lock lock{_mutex};

        _map.insert({id, value});
        return id;
    }

    /// Thread-safe.
    /// Destroys the object and removes it from the registry.
    /// From then on, trying to get an object using `id` is still safe but will return null.
    void destroy(const Id<T>& id)
    {
        std::unique_lock lock{_mutex};

        _map.erase(id);
    }

    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    [[nodiscard]] auto begin() { return _map.begin(); }
    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    [[nodiscard]] auto end() { return _map.end(); }
    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    [[nodiscard]] auto cbegin() const { return _map.cbegin(); }
    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    [[nodiscard]] auto cend() const { return _map.cend(); }
    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    [[nodiscard]] auto begin() const { return _map.begin(); }
    /// NOT Thread-safe; see the mutex() method to make this thread-safe.
    [[nodiscard]] auto end() const { return _map.end(); }

    /// Returns the mutex guarding this registry to allow you to lock it manually.
    /// This is only required when using functions that are not already thread-safe: get_ref(), get_mutable_ref(), begin(), end(), cbegin() and cend() (and therefore also using a range-based for loop on this registry).
    /// You should use a std::unique_lock if you want to modify some values, and std::shared_lock if you only need to read them.
    /// See https://stackoverflow.com/a/46050121/15432269 for more details about shared mutexes.
    [[nodiscard]] auto mutex() const -> std::shared_mutex& { return _mutex; }

    auto underlying_container() const -> const std::unordered_map<Id<T>, T>& { return _map; }
    auto underlying_container() -> std::unordered_map<Id<T>, T>& { return _map; }

private:
    std::unordered_map<Id<T>, T> _map;
    mutable std::shared_mutex    _mutex;
};

} // namespace reg
