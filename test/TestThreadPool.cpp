#include "ThreadPool.h"
#include "catch2/catch.hpp"

#include <iostream>

TEST_CASE("Try use threadpool", "[ThreadPool]")
{
    ThreadPool pool(4);
    std::vector<std::future<int>> results;

    for (int i = 0; i < 8; i++)
    {
        results.emplace_back(pool.enqueue([i] { return i * i; }));
    }

    std::vector<int> res;
    for (auto&& result : results)
        res.push_back(result.get());
    REQUIRE_THAT(res, Catch::Matchers::UnorderedEquals(std::vector<int> { 0, 1, 4, 9, 16, 25, 36, 49 }));
}