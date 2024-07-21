#pragma once
#include <ser20/types/memory.hpp>
#include <ser20/types/tuple.hpp>
#include <ser20/types/unordered_map.hpp>
#include <ser20/types/utility.hpp>
#include <ser20/types/variant.hpp>
#include <ser20/types/vector.hpp>
#include <stdexcept>
#include "reg.hpp"

namespace ser20 {

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
    archive(ser20::make_nvp("Underlying container", map.underlying_container()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::RawRegistry<T>& registry)
{
    archive(ser20::make_nvp("Underlying container", registry.underlying_container()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::RawOrderedRegistry<T>& registry)
{
    archive(ser20::make_nvp("Underlying container", registry.underlying_container().underlying_container()));
}

template<class Archive, typename T, typename Map>
void serialize(Archive& archive, reg::internal::RegistryImpl<T, Map>& registry)
{
    archive(ser20::make_nvp("Underlying registry", registry.underlying_wrapped_registry()));
}

template<class Archive, typename... Ts>
void serialize(Archive& archive, reg::Registries<Ts...>& registries)
{
    archive(ser20::make_nvp("Underlying registries", registries.underlying_registries()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::internal::IdDestroyer<T>& destroyer)
{
    archive(
        ser20::make_nvp("UUID", destroyer.underlying_uuid()),
        ser20::make_nvp("Registry", destroyer.underlying_registry())
    );
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::UniqueId<T>& id)
{
    archive(ser20::make_nvp("Underlying", id.underlying_object()));
}

template<class Archive, typename T>
void serialize(Archive& archive, reg::SharedId<T>& id)
{
    archive(ser20::make_nvp("Underlying", id.underlying_object()));
}

} // namespace ser20