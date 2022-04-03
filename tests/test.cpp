#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <odb/odb.hpp>

TEST_CASE("TEEEEST")
{
    auto database = odb::Database<float>{};

    auto id = database.insert(153.f);
    {
        const float* const value = database.get(id);
        REQUIRE(value);
        REQUIRE(*value == 153.f);
    }

    auto id2 = database.insert(10.f);
    {
        const float* const value = database.get(id);
        REQUIRE(value);
        REQUIRE(*value == 153.f);
        const float* const value2 = database.get(id2);
        REQUIRE(value2);
        REQUIRE(*value2 == 10.f);
    }
}