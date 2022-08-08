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

TEST_CASE_TEMPLATE("Trying to erase an uninitialized id is valid and does nothing", Registry, reg::Registry<char>, reg::OrderedRegistry<char>)
{
    auto       registry = Registry{};
    const auto idA      = registry.create('a');
    const auto idB      = registry.create('b');
    const auto idC      = registry.create('c');

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
    const auto id11      = registry1.create(2.f);
    const auto id12      = registry1.create(2.f);
    const auto id13      = registry1.create(1.f);
    const auto id21      = registry2.create(1.f);
    const auto id22      = registry2.create(1.f);
    const auto id23      = registry2.create(2.f);

    REQUIRE(id11 == id11);
    REQUIRE(id11 != id12);
    REQUIRE(id11 != id13);
    REQUIRE(id12 != id13);
    REQUIRE(id21 != id22);
    REQUIRE(id21 != id23);
    REQUIRE(id22 != id23);

    REQUIRE(id11 != id21);
    REQUIRE(id11 != id22);
    REQUIRE(id11 != id23);
    REQUIRE(id12 != id21);
    REQUIRE(id12 != id22);
    REQUIRE(id12 != id23);
    REQUIRE(id13 != id21);
    REQUIRE(id13 != id22);
    REQUIRE(id13 != id23);
}

TEST_CASE_TEMPLATE("An AnyId is equal to the Id it was created from", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    const auto id1      = registry.create(1.f);
    const auto id2      = registry.create(2.f);
    const auto any_id1  = reg::AnyId{id1};
    const auto any_id2  = reg::AnyId{id2};

    REQUIRE(id1 == any_id1);
    REQUIRE(any_id1 == id1);
    REQUIRE(id2 == any_id2);
    REQUIRE(id1 != any_id2);
    REQUIRE(any_id1 != id2);
    REQUIRE(any_id1 != any_id2);
    REQUIRE(!(any_id1 == any_id2));
}

TEST_CASE_TEMPLATE("Getting an object", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    const auto id       = registry.create(17.f);

    SUBCASE("get()")
    {
        const auto value = registry.get(id);

        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
    SUBCASE("with_ref()")
    {
        const bool object_has_been_found_in_registry = registry.with_ref(id, [](const float& value) {
            REQUIRE(value == 17.f);
        });
        REQUIRE(object_has_been_found_in_registry);
    }
    SUBCASE("with_mutable_ref()")
    {
        const bool object_has_been_found_in_registry = registry.with_mutable_ref(id, [](float& value) {
            REQUIRE(value == 17.f);
        });
        REQUIRE(object_has_been_found_in_registry);
    }
    SUBCASE("get_ref()")
    {
        std::shared_lock  lock{registry.mutex()};
        const auto* const value = registry.get_ref(id);

        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
    SUBCASE("get_mutable_ref()")
    {
        std::unique_lock lock{registry.mutex()};
        auto* const      value = registry.get_mutable_ref(id);

        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
}

TEST_CASE_TEMPLATE("Setting an object", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    const auto id       = registry.create(17.f);

    SUBCASE("set()")
    {
        const bool success = registry.set(id, 13.f);
        REQUIRE(success);
        REQUIRE(*registry.get(id) == 13.f);
    }
    SUBCASE("with_mutable_ref()")
    {
        const bool success = registry.with_mutable_ref(id, [](float& value) {
            value = 13.f;
        });
        REQUIRE(success);
        REQUIRE(*registry.get(id) == 13.f);
    }
    SUBCASE("get_mutable_ref()")
    {
        {
            std::unique_lock lock{registry.mutex()};
            auto* const      value = registry.get_mutable_ref(id);
            *value                 = 13.f;
        }
        REQUIRE(*registry.get(id) == 13.f);
    }
}

TEST_CASE_TEMPLATE("Objects can be created, retrieved and destroyed", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto registry = Registry{};

    const auto id1 = registry.create(153.f);
    REQUIRE(size(registry) == 1);
    {
        const std::optional<float> value1 = registry.get(id1);
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
    }

    const auto id2 = registry.create(10.f);
    REQUIRE(id2 != id1);
    REQUIRE(size(registry) == 2);
    {
        const auto value1 = registry.get(id1);
        const auto value2 = registry.get(id2);
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }

    registry.destroy(id1);
    REQUIRE(size(registry) == 1);
    {
        const auto value1 = registry.get(id1);
        const auto value2 = registry.get(id2);
        REQUIRE(!value1);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }
    registry.destroy(id1);
    REQUIRE(size(registry) == 1);
    {
        const auto value1 = registry.get(id1);
        const auto value2 = registry.get(id2);
        REQUIRE(!value1);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }
}

TEST_CASE_TEMPLATE("You can iterate over the ids and values in the registry", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto       registry = Registry{};
    const auto my_value = 1.f;
    const auto my_id    = registry.create(my_value);

    std::shared_lock lock{registry.mutex()};

    for (const auto& [id, value] : registry)
    {
        assert(id == my_id);       // Can't use doctest's REQUIRE() because of a weird interaction between lambda captures and structured bindings :'( https://github.com/doctest/doctest/issues/279
        assert(value == my_value); // Same
        std::ignore id;
        std::ignore value;
    }
}

TEST_CASE_TEMPLATE("Locking manually", Registry, reg::Registry<std::vector<float>>, reg::OrderedRegistry<std::vector<float>>)
{
    auto       registry = Registry{};                                          // Our registry is storing big objects
    const auto id       = registry.create(std::vector<float>(10000000, 15.f)); // so we will want to avoid copying them
    {
        std::shared_lock lock{registry.mutex()}; // I only want to read so I can use a shared_lock

        const auto* const vec_ref = registry.get_ref(id);
        REQUIRE(vec_ref);
        REQUIRE(vec_ref->size() == 10000000);
        REQUIRE((*vec_ref)[0] == 15.f);
    }
    {
        std::unique_lock lock{registry.mutex()}; // I want to modify so I need a unique_lock

        auto* const vec_mut_ref = registry.get_mutable_ref(id);
        REQUIRE(vec_mut_ref);
        (*vec_mut_ref)[17] = 21.f; // Do mutation
        REQUIRE((*registry.get_ref(id))[17] == 21.f);
    }

    const auto id2 = registry.create(std::vector<float>(20, 21.f));

    {
        std::shared_lock lock{registry.mutex()}; // I only want to read so I can use a shared_lock

        for (const auto& kv : registry)
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

        REQUIRE((*registry.get_ref(id))[0] == 1.f);
        REQUIRE((*registry.get_ref(id2))[0] == 1.f);
    }
}

TEST_CASE("Registries allows you to access the underlying registries by type")
{
    using Registries = reg::Registries<int, float, double>;
    Registries registries{};

    {
        reg::Registry<int>&       registry       = registries.of<int>();
        const reg::Id<int>        id             = registry.create(3);
        const reg::Registry<int>& const_registry = registries.of<int>();
        REQUIRE(const_registry.get(id) == 3);
    }

    {
        reg::Registry<float>&       registry       = registries.of<float>();
        const reg::Id<float>        id             = registry.create(3.f);
        const reg::Registry<float>& const_registry = registries.of<float>();
        REQUIRE(const_registry.get(id) == 3.f);
    }

    {
        reg::Registry<double>&       registry       = registries.of<double>();
        const reg::Id<double>        id             = registry.create(3.);
        const reg::Registry<double>& const_registry = registries.of<double>();
        REQUIRE(const_registry.get(id) == 3.);
    }
}

TEST_CASE_TEMPLATE("Registries expose the thread-safe functions of the underlying registries", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    using Registries = reg::Registries<float, int, double>;
    Registries registries{};
    {
        const auto id = registries.create(5);
        REQUIRE(registries.get(id) == 5);
        registries.set(id, 7);
        REQUIRE(registries.get(id) == 7);
        registries.destroy(id);
        REQUIRE(!registries.get(id));
    }
}

TEST_CASE_TEMPLATE("is_empty()", Registry, reg::Registry<float>, reg::OrderedRegistry<float>)
{
    auto registry = Registry{};
    CHECK(registry.is_empty());
    const auto id = registry.create(3.f);
    CHECK(!registry.is_empty());
    registry.destroy(id);
    CHECK(registry.is_empty());
}

TEST_CASE_TEMPLATE(
    "ScopedId", RegistryAndId,
    std::tuple<reg::Registry<float>, reg::ScopedId<float>>, // We use this tuple trick to run the test on pairs of types (the Registry and the corresponding ScopedId)
    std::tuple<reg::OrderedRegistry<float>, reg::ScopedId_Ordered<float>>
)
{
    using Registry = typename std::tuple_element<0, RegistryAndId>::type;
    using ScopedId = typename std::tuple_element<1, RegistryAndId>::type;

    auto registry = Registry{};
    REQUIRE(registry.is_empty());

    SUBCASE("The destructor of ScopedId automatically deletes the id it was responsible for.")
    {
        {
            const auto scoped_id = ScopedId{registry, 3.f};
            REQUIRE(*registry.get(scoped_id) == 3.f);
        }
        CHECK(registry.is_empty());
    }

#pragma warning(disable : 4068) // "unknown pragma"
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpessimizing-move"
#pragma GCC diagnostic   push
#pragma GCC diagnostic   ignored "-Wpessimizing-move"

    SUBCASE("Move-assigning a ScopedId transfers responsibility.")
    {
        {
            auto final_scope = ScopedId{};
            {
                auto tmp_scope = ScopedId{registry, 3.f};
                REQUIRE(*registry.get(tmp_scope) == 3.f);
                final_scope = std::move(tmp_scope);
            } // Destructor of tmp_scope is called but shouldn't do anything
            REQUIRE(*registry.get(final_scope) == 3.f);
        } // Destructor of final_scope is called and should destroy the id
        CHECK(registry.is_empty());
    }

    SUBCASE("Move-constructing a ScopedId transfers responsibility.")
    {
        {
            const auto final_scoped_id = [&]() {
                auto tmp_scoped_id = ScopedId{registry, 3.f}; // Can't be const if we want to move from it
                REQUIRE(*registry.get(tmp_scoped_id) == 3.f);
                return std::move(tmp_scoped_id); // Force a move, don't rely on copy-elision as this is not what we want to test
            }();                                 // Destructor of tmp_scope is called but shouldn't do anything
            REQUIRE(*registry.get(final_scoped_id) == 3.f);
        } // Destructor of final_scope is called and should destroy the id
        CHECK(registry.is_empty());
    }

#pragma GCC diagnostic   pop
#pragma clang diagnostic pop
}