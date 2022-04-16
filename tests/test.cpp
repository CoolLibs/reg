#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cassert>
#include <reg/reg.hpp>

template<typename T>
auto size(const reg::Registry<T>& registry)
{
    std::shared_lock lock{registry.mutex()}; // Not really needed since our tests are single-threaded, but it is an opportunity
                                             // to show what a thread-safe `size(Registry)` function would look like.
    return static_cast<size_t>(
        std::distance(std::begin(registry),
                      std::end(registry)));
}

TEST_CASE("Querying a registry with an uninitialized id returns a null object")
{
    auto registry = reg::Registry<int>{};
    REQUIRE(!registry.get(reg::Id<int>{}));
    REQUIRE(!registry.get_ref(reg::Id<int>{}));
    REQUIRE(!registry.get_mutable_ref(reg::Id<int>{}));
}

TEST_CASE("Trying to erase an uninitialized id is valid and does nothing")
{
    auto       registry = reg::Registry<char>{};
    const auto idA      = registry.create('a');
    const auto idB      = registry.create('b');
    const auto idC      = registry.create('c');

    registry.destroy(reg::Id<char>{});

    REQUIRE(size(registry) == 3);
    REQUIRE(*registry.get(idA) == 'a');
    REQUIRE(*registry.get(idB) == 'b');
    REQUIRE(*registry.get(idC) == 'c');
}


TEST_CASE("An AnyId is equal to the Id it was created from")
{
    auto       registry = reg::Registry<float>{};
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
TEST_CASE("Getting an object")
{
    auto       registry = reg::Registry<float>{};
    const auto id       = registry.create(17.f);

    SUBCASE("get()")
    {
        const auto value = registry.get(id);
        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
    SUBCASE("get_ref()")
    {
        std::shared_lock lock{registry.mutex()};

        const auto* const value = registry.get_ref(id);
        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
    SUBCASE("get_mutable_ref()")
    {
        std::unique_lock lock{registry.mutex()};

        auto* const value = registry.get_mutable_ref(id);
        REQUIRE(value);
        REQUIRE(*value == 17.f);
    }
}

TEST_CASE("Objects can be created, retrieved and destroyed")
{
    auto registry = reg::Registry<float>{};

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

TEST_CASE("You can iterate over the ids and values in the registry")
{
    auto       registry = reg::Registry<float>{};
    const auto my_value = 1.f;
    const auto my_id    = registry.create(my_value);

    std::shared_lock lock{registry.mutex()};

    for (const auto& [id, value] : registry) {
        assert(id == my_id);       // Can't use doctest's REQUIRE() because of a weird interaction between lambda captures and structured bindings :'( https://github.com/doctest/doctest/issues/279
        assert(value == my_value); // Same
    }
}

TEST_CASE("Locking manually")
{
    auto       registry = reg::Registry<std::vector<float>>{};                 // Our registry is storing big objects
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

        for (const auto& kv : registry) {
            REQUIRE(kv.second[0] > 0.f); // Some silly thing, this is just an example of how you would read values
        }
    }
    {
        std::unique_lock lock{registry.mutex()}; // I want to modify so I need a unique_lock

        for (auto& kv : registry) {
            kv.second[0] = 1.f;
        }

        REQUIRE((*registry.get_ref(id))[0] == 1.f);
        REQUIRE((*registry.get_ref(id2))[0] == 1.f);
    }
}