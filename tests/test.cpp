#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <odb/odb.hpp>

class FloatId : public odb::Id<float> {
};

TEST_CASE("TEEEEST")
{
    auto database = odb::Database<FloatId>{};

    auto id = database.insert(153.f);
    {
        auto value = database.get(id);
        REQUIRE(value.has_value());
        REQUIRE(*value == 153.f);
    }

    auto id2 = database.insert(10.f);
    {
        auto value = database.get(id);
        REQUIRE(value.has_value());
        REQUIRE(*value == 153.f);
        auto value2 = database.get(id2);
        REQUIRE(value2.has_value());
        REQUIRE(*value2 == 10.f);
    }
}