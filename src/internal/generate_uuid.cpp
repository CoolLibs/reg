#include "generate_uuid.hpp"
#include <array>
#include <random>

namespace reg::internal {

static auto create_random_generator()
{
    auto rd        = std::random_device{};
    auto seed_data = std::array<int, std::mt19937::state_size>{};
    std::generate(std::begin(seed_data), std::end(seed_data), std::ref(rd));
    auto seq = std::seed_seq(std::begin(seed_data), std::end(seed_data));
    return std::mt19937{seq};
}

auto generate_uuid() -> uuids::uuid
{
    static thread_local auto rng            = create_random_generator();
    static thread_local auto uuid_generator = uuids::uuid_random_generator{rng};
    return uuid_generator();
}

} // namespace reg::internal