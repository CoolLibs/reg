#pragma once
#include <uuid.h>
#include <unordered_map>
#include "Id.hpp"

namespace reg {

template<typename T>
class Registry {
public:
    using ValueType = T;

    Registry()           = default;
    Registry(Registry&&) = default;
    Registry& operator=(Registry&&) = default;

    Registry(const Registry&) = delete;            // This class is non-copyable
    Registry& operator=(const Registry&) = delete; // because it is the unique owner of the objects it stores

    [[nodiscard]] auto get(const Id<T>& id) -> T*
    {
        auto it = _map.find(id);
        if (it == _map.end()) {
            return nullptr;
        }
        else {
            return &it->second;
        }
    }

    [[nodiscard]] auto get(const Id<T>& id) const -> const T*
    {
        const auto it = _map.find(id);
        if (it == _map.end()) {
            return nullptr;
        }
        else {
            return &it->second;
        }
    }

    [[nodiscard]] auto create(const T& value) -> Id<T>
    {
        const auto uuid = uuids::uuid_system_generator{}(); // This should be thread-safe on all OSes // Creating a uuid_system_generator is cheap, this class is empty.
        const auto id   = Id<T>{uuid};
        _map.insert({id, value});
        return id;
    }

    void destroy(const Id<T>& id)
    {
        _map.erase(id);
    }

    auto begin() { return _map.begin(); }
    auto end() { return _map.end(); }
    auto cbegin() const { return _map.cbegin(); }
    auto cend() const { return _map.cend(); }
    auto begin() const { return _map.begin(); }
    auto end() const { return _map.end(); }

private:
    std::unordered_map<Id<T>, T> _map;
};

} // namespace reg
