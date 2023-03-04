#pragma once
#include <mutex>
#include <optional>
#include <shared_mutex>
#include "../Id.hpp"
#include "generate_uuid.hpp"

namespace reg::internal {

template<typename T, typename Map>
class RawRegistryImpl {
public:
    /// The type of values stored in this registry.
    using ValueType = T;

    RawRegistryImpl()                                              = default;
    ~RawRegistryImpl()                                             = default;
    RawRegistryImpl(RawRegistryImpl&&) noexcept                    = default;
    auto operator=(RawRegistryImpl&&) noexcept -> RawRegistryImpl& = default;

    RawRegistryImpl(RawRegistryImpl const&)                    = delete; // This class is non-copyable
    auto operator=(RawRegistryImpl const&) -> RawRegistryImpl& = delete; // because it is the unique owner of the objects it stores

    [[nodiscard]] auto get(Id<T> const& id) const -> std::optional<T>
    {
        std::shared_lock lock{_mutex};

        auto const it = _map.find(id);
        if (it == _map.end())
            return std::nullopt;

        return it->second;
    }

    auto set(Id<T> const& id, T const& value) -> bool
    {
        std::unique_lock lock{_mutex};

        auto it = _map.find(id);
        if (it == _map.end())
            return false;

        it->second = value;
        return true;
    }

    [[nodiscard]] auto contains(Id<T> const& id) const -> bool
    {
        std::shared_lock lock{_mutex};

        auto const it = _map.find(id);
        return it != _map.end();
    }

    auto with_ref(Id<T> const& id, std::function<void(T const&)> const& callback) const -> bool
    {
        std::shared_lock lock{_mutex};

        auto const it = _map.find(id);
        if (it == _map.end())
            return false;

        callback(it->second);
        return true;
    }

    auto with_mutable_ref(Id<T> const& id, std::function<void(T&)> const& callback) -> bool
    {
        std::unique_lock lock{_mutex};

        auto const it = _map.find(id);
        if (it == _map.end())
            return false;

        callback(it->second);
        return true;
    }

    [[nodiscard]] auto get_ref(Id<T> const& id) const -> T const*
    {
        auto const it = _map.find(id);
        if (it == _map.end())
            return nullptr;

        return &it->second;
    }

    [[nodiscard]] auto get_mutable_ref(Id<T> const& id) -> T*
    {
        auto it = _map.find(id);
        if (it == _map.end())
            return nullptr;

        return &it->second;
    }

    [[nodiscard]] auto create_raw(T const& value) -> Id<T>
    {
        auto const       id = Id<T>{generate_uuid()};
        std::unique_lock lock{_mutex};
        _map.insert({id, value});
        return id;
    }

    void destroy(Id<T> const& id)
    {
        std::unique_lock lock{_mutex};

        _map.erase(id);
    }

    [[nodiscard]] auto is_empty() const -> bool
    {
        std::shared_lock lock{_mutex};
        return _map.empty();
    }

    void clear()
    {
        std::unique_lock lock{_mutex};
        _map.clear();
    }

    [[nodiscard]] auto begin() { return _map.begin(); }
    [[nodiscard]] auto end() { return _map.end(); }
    [[nodiscard]] auto begin() const { return _map.begin(); }
    [[nodiscard]] auto end() const { return _map.end(); }
    [[nodiscard]] auto cbegin() const { return _map.cbegin(); }
    [[nodiscard]] auto cend() const { return _map.cend(); }

    [[nodiscard]] auto mutex() const -> std::shared_mutex& { return _mutex; }

    [[nodiscard]] auto underlying_container() const -> Map const& { return _map; }
    [[nodiscard]] auto underlying_container() -> Map& { return _map; }

private:
    Map                       _map;
    mutable std::shared_mutex _mutex;
};

} // namespace reg::internal