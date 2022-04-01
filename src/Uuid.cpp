#include "Uuid.hpp"
#include <chrono>
#include <limits>
#include <random>
#include <thread>

namespace odb::internal {

/// Creates a random number generator with a seed that will be different for each thread
static std::mt19937 create_rng()
{
    std::seed_seq seeds{std::hash<std::thread::id>{}(std::this_thread::get_id()),                                   // thread::get_id() is unique for each std::thread and std::jthread, but if a thread finishes then the value can be reused for another thread
                        static_cast<size_t>(std::chrono::high_resolution_clock::now().time_since_epoch().count())}; // this is why we also take the time into account for our seed
    return std::mt19937{seeds};
}

static Uuid::StorageType random_uuid()
{
    static thread_local std::mt19937 generator{create_rng()};

    static thread_local std::uniform_int_distribution<Uuid::StorageType> distribution(
        std::numeric_limits<Uuid::StorageType>::min(),
        std::numeric_limits<Uuid::StorageType>::max());

    return distribution(generator);
}

Uuid::Uuid()
    : _uuid{random_uuid()}
{
}

} // namespace odb::internal
