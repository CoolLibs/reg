#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <reg/reg.hpp>

TEST_CASE("TEEEEST")
{
    auto registry = reg::Registry<float>{};

    auto id = registry.insert(153.f);
    {
        const float* const value = registry.get(id);
        REQUIRE(value);
        REQUIRE(*value == 153.f);
    }

    auto id2 = registry.insert(10.f);
    {
        const float* const value = registry.get(id);
        REQUIRE(value);
        REQUIRE(*value == 153.f);
        const float* const value2 = registry.get(id2);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }
}