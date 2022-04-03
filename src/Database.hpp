#pragma once
#include <optional>
#include <unordered_map>
#include "Id.hpp"
#include "Uuid.hpp"

namespace odb {

template<typename T>
class Database {
public:
    using ValueType = T;

    auto get(const Id<T>& id) const -> std::optional<T>
    {
        if (!id._uuid.has_value()) {
            return std::nullopt;
        }

        const auto it = _map.find(id._uuid->get());
        if (it != _map.end()) {
            return it->second;
        }
        else {
            return std::nullopt;
        }
    }

    [[nodiscard]] auto insert(T value) -> Id<T>
    {
        const auto uuid = internal::Uuid{};
        const auto id   = Id<T>{uuid};
        _map.insert(std::make_pair(uuid.get(), value));
        return id;
    }

private:
    std::unordered_map<typename internal::Uuid::StorageType, T> _map;
};

} // namespace odb
