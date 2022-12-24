#pragma once

#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include "reg.hpp"

namespace cereal {

template<class Archive>
auto save_minimal(Archive const&, uuids::uuid const& uuid) -> std::string
{
    return uuids::to_string(uuid);
}

template<class Archive>
void load_minimal(Archive const&, uuids::uuid& uuid, std::string const& value)
{
    auto const maybe_uuid = uuids::uuid::from_string(value);
    if (!maybe_uuid)
        throw std::runtime_error{"[load(uuids::uuid)] Couldn't parse uuid: " + value};

    uuid = *maybe_uuid;
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::Id<T>& id)
{
    archive(id.underlying_uuid());
}

template<class Archive>
void serialize(Archive& archive, reg::AnyId& id)
{
    archive(id.underlying_uuid());
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::Registry<T>& registry)
{
    archive(registry.underlying_container());
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::OrderedRegistry<T>& registry)
{
    archive(registry.underlying_container());
}

template<class Archive, typename Key, typename Value>
void serialize(Archive& archive, reg::internal::OrderPreservingMap<Key, Value>& map)
{
    archive(map.underlying_container());
}

template<class Archive, typename... Ts>
void serialize(Archive& archive, reg::Registries<Ts...>& registries)
{
    archive(registries.underlying_registries());
}

} // namespace cereal