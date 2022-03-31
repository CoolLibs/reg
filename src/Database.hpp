#pragma once
#include <optional>
#include <unordered_map>
#include "Uuid.hpp"

namespace odb {

template<typename Id>
class Database {
public:
    auto get(const Id& id) const -> std::optional<typename Id::ValueType>
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

    [[nodiscard]] auto insert(typename Id::ValueType value) -> Id
    {
        const auto uuid = internal::Uuid{};
        const auto id   = Id{uuid};
        _map.insert(std::make_pair(uuid.get(), value));
        return id;
    }

private:
    std::unordered_map<typename internal::Uuid::StorageType, typename Id::ValueType> _map;
};

} // namespace odb
