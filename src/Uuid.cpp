#pragma once
#include "Uuid.hpp"
#include <time.h>
#include <limits>
#include <random>
#include <thread>

namespace odb::internal {

static auto random_uuid()
{
    static thread_local std::mt19937 generator{0 /*std::clock()+ std::this_thread::get_id()._Id*/}; // TODO

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
