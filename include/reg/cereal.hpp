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
auto save_minimal(Archive const& ar, reg::RawId<T> const& id) -> std::string
{
    return save_minimal(ar, id.underlying_uuid());
}
template<class Archive, typename T>
void load_minimal(Archive const& ar, reg::RawId<T>& id, std::string const& value)
{
    load_minimal(ar, id.underlying_uuid(), value);
}

template<class Archive>
auto save_minimal(Archive const& ar, reg::AnyId const& id) -> std::string
{
    return save_minimal(ar, id.underlying_uuid());
}
template<class Archive>
void load_minimal(Archive const& ar, reg::AnyId& id, std::string const& value)
{
    load_minimal(ar, id.underlying_uuid(), value);
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::Registry<T>& registry)
{
    archive(cereal::make_nvp("Underlying container", registry.underlying_container()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::OrderedRegistry<T>& registry)
{
    archive(cereal::make_nvp("Underlying container", registry.underlying_container().underlying_container()));
}

template<class Archive, typename... Ts>
void serialize(Archive& archive, reg::Registries<Ts...>& registries)
{
    archive(cereal::make_nvp("Underlying registries", registries.underlying_registries()));
}

} // namespace cereal