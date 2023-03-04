#pragma once

#include <cereal/types/memory.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/variant.hpp>
#include <cereal/types/vector.hpp>
#include <stdexcept>
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
auto save_minimal(Archive const& ar, reg::Id<T> const& id) -> std::string
{
    return save_minimal(ar, id.underlying_uuid());
}
template<class Archive, typename T>
void load_minimal(Archive const& ar, reg::Id<T>& id, std::string const& value)
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

template<class Archive, typename T, typename Map>
void serialize(Archive& archive, reg::internal::OrderPreservingMap<T, Map>& map)
{
    archive(cereal::make_nvp("Underlying container", map.underlying_container()));
}

template<class Archive, typename T, typename Map>
void serialize(Archive& archive, reg::internal::RawRegistryImpl<T, Map>& registry)
{
    archive(cereal::make_nvp("Underlying container", registry.underlying_container()));
}

template<class Archive, typename T, typename Map>
void serialize(Archive& archive, reg::internal::RegistryImpl<T, Map>& registry)
{
    archive(cereal::make_nvp("Underlying registry", registry.underlying_wrapped_registry()));
}

template<class Archive, typename... Ts>
void serialize(Archive& archive, reg::Registries<Ts...>& registries)
{
    archive(cereal::make_nvp("Underlying registries", registries.underlying_registries()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::internal::IdDestroyer<T>& destroyer)
{
    archive(
        cereal::make_nvp("UUID", destroyer.underlying_uuid()),
        cereal::make_nvp("Registry", destroyer.underlying_registry())
    );
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::UniqueId<T>& id)
{
    archive(cereal::make_nvp("Underlying", id.underlying_object()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::SharedId<T>& id)
{
    archive(cereal::make_nvp("Underlying", id.underlying_object()));
}

} // namespace cereal