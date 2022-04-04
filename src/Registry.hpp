#pragma once
#include <uuid.h>
#include <unordered_map>
#include "Id.hpp"

namespace reg {

template<typename T>
class Registry {
public:
    using ValueType = T;

    auto get(const Id<T>& id) -> T*
    {
        if (id._uuid.is_nil()) {
            return nullptr;
        }
        auto it = _map.find(id._uuid);
        if (it != _map.end()) {
            return &it->second;
        }
        else {
            return nullptr;
        }
    }

    auto get(const Id<T>& id) const -> const T*
    {
        if (id._uuid.is_nil()) {
            return nullptr;
        }
        const auto it = _map.find(id._uuid->get());
        if (it != _map.end()) {
            return &it->second;
        }
        else {
            return nullptr;
        }
    }

    [[nodiscard]] auto insert(T&& value) -> Id<T>
    {
        const auto uuid = uuids::uuid_system_generator{}();
        const auto id   = Id<T>{uuid};
        _map.insert(std::make_pair(uuid, value));
        return id;
    }

private:
    std::unordered_map<uuids::uuid, T> _map;
};

} // namespace reg
