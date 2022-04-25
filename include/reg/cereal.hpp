#pragma once

#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include "reg.hpp"

namespace cereal {

template<class Archive>
void save(Archive& ar, const uuids::uuid& uuid)
{
    const std::string s = uuids::to_string(uuid);
    ar(s);
}

template<class Archive>
void load(Archive& ar, uuids::uuid& uuid)
{
    std::string s;
    ar(s);
    const auto maybe_uuid = uuids::uuid::from_string(s);
    if (maybe_uuid) {
        uuid = *maybe_uuid;
    }
    else {
        throw std::runtime_error{"[load(uuids::uuid)] Couldn't parse uuid: " + s};
    }
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::Id<T>& id)
{
    archive(id.underlying_uuid());
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::AnyId& id)
{
    archive(id.underlying_uuid());
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::Registry<T>& registry)
{
    archive(registry.underlying_container());
}

template<class Archive, typename... Ts>
void serialize(Archive& archive, reg::Registries<Ts...>& registries)
{
    archive(registries.underlying_registries());
}

} // namespace cereal