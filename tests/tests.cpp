#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cassert>
#include <reg/reg.hpp>
#include <tuple>

template<typename Registry>
auto size(const Registry& registry)
{
    std::shared_lock lock{registry.mutex()}; // Not really needed since our tests are single-threaded, but it is an opportunity
                                             // to show what a thread-safe `size(Registry)` function would look like.
    return static_cast<size_t>(
        std::distance(
            std::begin(registry),
            std::end(registry)
        )
    );
}

TEST_CASE_TEMPLATE("Querying a registry with an uninitialized id returns a null object", Registry, reg::Registry<int>, reg::OrderedRegistry<int>)
{
    auto registry = Registry{};
    REQUIRE(!registry.get(reg::Id<int>{}));
    REQUIRE(!registry.get_ref(reg::Id<int>{}));
    REQUIRE(!registry.get_mutable_ref(reg::Id<int>{}));
}

TEST_CASE_TEMPLATE("Trying to erase an uninitialized id is valid and does nothing", RawRegistry, reg::internal::RawRegistry<char, std::unordered_map<reg::Id<char>, char>>, reg::internal::RawRegistry<char, reg::internal::OrderPreservingMap<reg::Id<char>, char>>)
{
    auto       registry = RawRegistry{};
    auto const idA      = registry.create_raw('a');
    auto const idB      = registry.create_raw('b');
    auto const idC      = registry.create_raw('c');

    registry.destroy(reg::Id<char>{});

    REQUIRE(size(registry) == 3);
    REQUIRE(*registry.get(idA) == 'a');
    REQUIRE(*registry.get(idB) == 'b');
    REQUIRE(*registry.get(idC) == 'c');
}

TEST_CASE_TEMPLATE("IDs are unique, even across registries", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry1 = Registry{};
    auto       registry2 = Registry{};
    auto const id11      = registry1.create_unique(2.f);
    auto const id12      = registry1.create_unique(2.f);
    auto const id13      = registry1.create_unique(1.f);
    auto const id21      = registry2.create_unique(1.f);
    auto const id22      = registry2.create_unique(1.f);
    auto const id23      = registry2.create_unique(2.f);

    REQUIRE(id11.raw() == id11.raw());
    REQUIRE(id11.raw() != id12.raw());
    REQUIRE(id11.raw() != id13.raw());
    REQUIRE(id12.raw() != id13.raw());
    REQUIRE(id21.raw() != id22.raw());
    REQUIRE(id21.raw() != id23.raw());
    REQUIRE(id22.raw() != id23.raw());

    REQUIRE(id11.raw() != id21.raw());
    REQUIRE(id11.raw() != id22.raw());
    REQUIRE(id11.raw() != id23.raw());
    REQUIRE(id12.raw() != id21.raw());
    REQUIRE(id12.raw() != id22.raw());
    REQUIRE(id12.raw() != id23.raw());
    REQUIRE(id13.raw() != id21.raw());
    REQUIRE(id13.raw() != id22.raw());
    REQUIRE(id13.raw() != id23.raw());
}

TEST_CASE_TEMPLATE("An AnyId is equal to the Id it was created from", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    auto const id1      = registry.create_unique(1.f);
    auto const id2      = registry.create_unique(2.f);
    auto const any_id1  = reg::AnyId{id1.raw()};
    auto const any_id2  = reg::AnyId{id2.raw()};

    REQUIRE(id1.raw() == any_id1);
    REQUIRE(any_id1 == id1.raw());
    REQUIRE(id2.raw() == any_id2);
    REQUIRE(id1.raw() != any_id2);
    REQUIRE(any_id1 != id2.raw());
    REQUIRE(any_id1 != any_id2);
    REQUIRE(!(any_id1 == any_id2));
}

TEST_CASE_TEMPLATE("Getting an object", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    auto const id       = registry.create_unique(17.f);

    SUBCASE("get()")
    {
        auto const value = registry.get(id.raw());

        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
    SUBCASE("with_ref()")
    {
        const bool object_has_been_found_in_registry = registry.with_ref(id.raw(), [](const float& value) {
            REQUIRE(value == 17.f);
        });
        REQUIRE(object_has_been_found_in_registry);
    }
    SUBCASE("with_mutable_ref()")
    {
        const bool object_has_been_found_in_registry = registry.with_mutable_ref(id.raw(), [](float& value) {
            REQUIRE(value == 17.f);
        });
        REQUIRE(object_has_been_found_in_registry);
    }
    SUBCASE("get_ref()")
    {
        std::shared_lock  lock{registry.mutex()};
        auto const* const value = registry.get_ref(id.raw());

        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
    SUBCASE("get_mutable_ref()")
    {
        std::unique_lock lock{registry.mutex()};
        auto* const      value = registry.get_mutable_ref(id.raw());

        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
}

TEST_CASE_TEMPLATE("Setting an object", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    auto const id       = registry.create_unique(17.f);

    SUBCASE("set()")
    {
        const bool success = registry.set(id.raw(), 13.f);
        REQUIRE(success);
        REQUIRE(*registry.get(id.raw()) == 13.f);
    }
    SUBCASE("with_mutable_ref()")
    {
        const bool success = registry.with_mutable_ref(id.raw(), [](float& value) {
            value = 13.f;
        });
        REQUIRE(success);
        REQUIRE(*registry.get(id.raw()) == 13.f);
    }
    SUBCASE("get_mutable_ref()")
    {
        {
            std::unique_lock lock{registry.mutex()};
            auto* const      value = registry.get_mutable_ref(id.raw());
            *value                 = 13.f;
        }
        REQUIRE(*registry.get(id.raw()) == 13.f);
    }
}

TEST_CASE_TEMPLATE("Objects can be created and retrieved", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto registry = Registry{};

    auto const id1 = registry.create_unique(153.f);
    REQUIRE(size(registry) == 1);
    {
        const std::optional<float> value1 = registry.get(id1.raw());
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
    }

    auto const id2 = registry.create_unique(10.f);
    REQUIRE(id2.raw() != id1.raw());
    REQUIRE(size(registry) == 2);
    {
        auto const value1 = registry.get(id1.raw());
        auto const value2 = registry.get(id2.raw());
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }
}

TEST_CASE_TEMPLATE("You can iterate over the ids and values in the registry", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    auto const my_value = 1.f;
    auto const my_id    = registry.create_unique(my_value);

    std::shared_lock lock{registry.mutex()};

    for (auto const& [id, value] : registry)
    {
        assert(id == my_id.raw()); // Can't use doctest's REQUIRE() because of a weird interaction between lambda captures and structured bindings :'( https://github.com/doctest/doctest/issues/279
        assert(value == my_value); // Same
        std::ignore = id;
        std::ignore = value;
    }
    std::ignore = my_id;
}

TEST_CASE_TEMPLATE("Locking manually", Registry, reg::Registry<std::vector<float>>, reg::OrderedRegistry<std::vector<float>>)
{
    auto       registry = Registry{};                                                 // Our registry is storing big objects
    auto const id       = registry.create_unique(std::vector<float>(10000000, 15.f)); // so we will want to avoid copying them
    {
        std::shared_lock lock{registry.mutex()}; // I only want to read so I can use a shared_lock

        auto const* const vec_ref = registry.get_ref(id.raw());
        REQUIRE(vec_ref);
        REQUIRE(vec_ref->size() == 10000000);
        REQUIRE((*vec_ref)[0] == 15.f);
    }
    {
        std::unique_lock lock{registry.mutex()}; // I want to modify so I need a unique_lock

        auto* const vec_mut_ref = registry.get_mutable_ref(id.raw());
        REQUIRE(vec_mut_ref);
        (*vec_mut_ref)[17] = 21.f; // Do mutation
        REQUIRE((*registry.get_ref(id.raw()))[17] == 21.f);
    }

    auto const id2 = registry.create_unique(std::vector<float>(20, 21.f));

    {
        std::shared_lock lock{registry.mutex()}; // I only want to read so I can use a shared_lock

        for (auto const& kv : registry)
        {
            REQUIRE(kv.second[0] > 0.f); // Some silly thing, this is just an example of how you would read values
        }
    }
    {
        std::unique_lock lock{registry.mutex()}; // I want to modify so I need a unique_lock

        for (auto& kv : registry)
        {
            kv.second[0] = 1.f;
        }

        REQUIRE((*registry.get_ref(id.raw()))[0] == 1.f);
        REQUIRE((*registry.get_ref(id2.raw()))[0] == 1.f);
    }
}

TEST_CASE("Registries allows you to access the underlying registries by type")
{
    using Registries = reg::Registries<
        reg::Registry<int>,
        reg::Registry<float>,
        reg::Registry<double>>;
    Registries registries{};

    {
        reg::Registry<int>&       registry       = registries.of<int>();
        reg::UniqueId<int> const  id             = registry.create_unique(3);
        reg::Registry<int> const& const_registry = registries.of<int>();
        REQUIRE(const_registry.get(id.raw()) == 3);
    }

    {
        reg::Registry<float>&       registry       = registries.of<float>();
        reg::UniqueId<float> const  id             = registry.create_unique(3.f);
        reg::Registry<float> const& const_registry = registries.of<float>();
        REQUIRE(const_registry.get(id.raw()) == 3.f);
    }

    {
        reg::Registry<double>&       registry       = registries.of<double>();
        reg::UniqueId<double> const  id             = registry.create_unique(3.);
        reg::Registry<double> const& const_registry = registries.of<double>();
        REQUIRE(const_registry.get(id.raw()) == 3.);
    }
}

TEST_CASE_TEMPLATE("Registries expose the thread-safe functions of the underlying registries", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    using Registries = reg::Registries<
        reg::Registry<float>,
        reg::Registry<int>,
        reg::Registry<double>>;
    Registries registries{};
    {
        auto const id = registries.create_unique(5);
        REQUIRE(registries.get(id.raw()) == 5);
        registries.set(id.raw(), 7);
        REQUIRE(registries.get(id.raw()) == 7);
    }
}

TEST_CASE_TEMPLATE("is_empty()", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto registry = Registry{};
    CHECK(registry.is_empty());
    {
        auto const id = registry.create_unique(3.f);
        CHECK(!registry.is_empty());
    }
    CHECK(registry.is_empty());
}

TEST_CASE_TEMPLATE("clear()", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto registry = Registry{};
    std::ignore   = registry.create_unique(3.f);
    std::ignore   = registry.create_unique(4.f);
    registry.clear();
    CHECK(size(registry) == 0);
}

TEST_CASE_TEMPLATE(
    "UniqueId", Registry,
    reg::Registry<float>,
    reg::OrderedRegistry<float>
)
{
    auto registry = Registry{};
    REQUIRE(registry.is_empty());

    SUBCASE("The destructor of UniqueId automatically deletes the id it was owning.")
    {
        {
            auto const unique_id = registry.create_unique(3.f);
            REQUIRE(*registry.get(unique_id.raw()) == 3.f);
        }
        CHECK(registry.is_empty());
    }

#pragma warning(disable : 4068) // "unknown pragma"
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wpessimizing-move"

    SUBCASE("Move-assigning a UniqueId transfers ownership.")
    {
        {
            auto final_scope = reg::UniqueId<float>{};
            {
                auto tmp_scope = registry.create_unique(3.f);
                REQUIRE(*registry.get(tmp_scope.raw()) == 3.f);
                final_scope = std::move(tmp_scope);
            } // Destructor of tmp_scope is called here but shouldn't do anything
            REQUIRE(*registry.get(final_scope.raw()) == 3.f);
        } // Destructor of final_scope is called here and should destroy the id
        CHECK(registry.is_empty());
    }

    SUBCASE("Move-constructing a UniqueId transfers ownership.")
    {
        {
            auto const final_scope = [&]() {
                auto tmp_scope = registry.create_unique(3.f); // Can't be const if we want to move from it
                REQUIRE(*registry.get(tmp_scope.raw()) == 3.f);
                return std::move(tmp_scope); // Force a move, don't rely on copy-elision as this is not what we want to test
            }();                             // Destructor of tmp_scope is called here but shouldn't do anything
            REQUIRE(*registry.get(final_scope.raw()) == 3.f);
        } // Destructor of final_scope is called here and should destroy the id
        CHECK(registry.is_empty());
    }

#pragma GCC diagnostic pop
}

#pragma warning(disable : 5054) // "operator '|': deprecated between enumerations of different types"
#pragma GCC diagnostic push
#pragma clang diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma clang diagnostic ignored "-Wimplicit-int-conversion"
#include <cereal/archives/json.hpp>
#pragma GCC diagnostic pop
#pragma clang diagnostic pop
#include <reg/cereal.hpp>
#include <sstream>

TEST_CASE_TEMPLATE("Serialization()", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    // Save
    auto                       registry  = Registry{};
    reg::Id<float> const       id        = registry.create_raw(3.f);
    reg::UniqueId<float> const unique_id = registry.create_unique(3.f);
    reg::SharedId<float> const shared_id = registry.create_shared(3.f);
    std::stringstream          ss{};
    {
        cereal::JSONOutputArchive out_archive{ss};
        out_archive(id, unique_id, shared_id);
    }

    // Load
    reg::Id<float>       out_id;
    reg::UniqueId<float> out_unique_id;
    reg::SharedId<float> out_shared_id;
    {
        cereal::JSONInputArchive in_archive{ss};
        in_archive(out_id, out_unique_id, out_shared_id);
    }

    // Check
    CHECK(id == out_id);
    CHECK(unique_id.raw() == out_unique_id.raw());
    CHECK(shared_id.raw() == out_shared_id.raw());
}