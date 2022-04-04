#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <cassert>
#include <reg/reg.hpp>

template<typename T>
auto objects_count(const reg::Registry<T>& registry) -> size_t
{
    return std::distance(std::begin(registry),
                         std::end(registry));
}

TEST_CASE("Querying a registry with a nil id returns a null object")
{
    auto registry = reg::Registry<int>{};
    REQUIRE(!registry.get(reg::Id<int>{}));
}

TEST_CASE("Trying to erase a nil ID is valid and does nothing")
{
    auto       registry = reg::Registry<char>{};
    const auto idA      = registry.create('a');
    const auto idB      = registry.create('b');
    const auto idC      = registry.create('c');

    registry.destroy(reg::Id<char>{});

    REQUIRE(objects_count(registry) == 3);
    REQUIRE(*registry.get(idA) == 'a');
    REQUIRE(*registry.get(idB) == 'b');
    REQUIRE(*registry.get(idC) == 'c');
}

TEST_CASE("Objects can be created, retrieved and destroyed")
{
    auto registry = reg::Registry<float>{};

    auto id1 = registry.create(153.f);
    REQUIRE(objects_count(registry) == 1);
    {
        const float* const value1 = registry.get(id1);
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
    }

    auto id2 = registry.create(10.f);
    REQUIRE(id2 != id1);
    REQUIRE(objects_count(registry) == 2);
    {
        const float* const value1 = registry.get(id1);
        const float* const value2 = registry.get(id2);
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }

    registry.destroy(id1);
    REQUIRE(objects_count(registry) == 1);
    {
        const float* const value1 = registry.get(id1);
        const float* const value2 = registry.get(id2);
        REQUIRE(!value1);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }
    registry.destroy(id1);
    REQUIRE(objects_count(registry) == 1);
    {
        const float* const value1 = registry.get(id1);
        const float* const value2 = registry.get(id2);
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

    for (const auto& [id, value] : registry) {
        assert(id == my_id);       // Can't use doctest's REQUIRE() because of a weird interaction between lambda captures and structured bindings :'( https://github.com/doctest/doctest/issues/279
        assert(value == my_value); // Same
    }
}