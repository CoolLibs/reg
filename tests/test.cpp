#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <reg/reg.hpp>

TEST_CASE("A default-constructed ID is nil")
{
    auto id = reg::Id<double>{};
    REQUIRE(id.is_nil());
}

TEST_CASE("Objects can be created, retrieved and destroyed")
{
    auto registry = reg::Registry<float>{};

    auto id1 = registry.create(153.f);
    REQUIRE(!id1.is_nil());
    {
        const float* const value1 = registry.get(id1);
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
    }

    auto id2 = registry.create(10.f);
    REQUIRE(!id2.is_nil());
    REQUIRE(id2 != id1);
    {
        const float* const value1 = registry.get(id1);
        const float* const value2 = registry.get(id2);
        REQUIRE(value1);
        REQUIRE(*value1 == 153.f);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }

    // {
    //     registry.destroy(id1);

    //     const float* const value1 = registry.get(id1);
    //     const float* const value2 = registry.get(id2);
    //     REQUIRE(!value1);
    //     REQUIRE(value2);
    //     REQUIRE(*value2 == 10.f);
    // }
}