#include <exe/runtime/multi_thread.hpp>

#include <exe/future/make/contract.hpp>
#include <exe/future/make/spawn.hpp>

#include <exe/result/make/err.hpp>
#include <exe/result/make/ok.hpp>

#include <exe/future/combine/seq/map.hpp>
#include <exe/future/combine/seq/via.hpp>

#include <exe/future/combine/concur/all.hpp>
#include <exe/future/combine/concur/first.hpp>

#include <exe/future/combine/concur/result/all.hpp>
#include <exe/future/combine/concur/result/first.hpp>

#include <exe/future/terminate/get.hpp>

#include <exe/unit.hpp>

// Testing

#include <twist/ed/std/thread.hpp>

#include <wheels/test/framework.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <fmt/core.h>

#include <atomic>
#include <chrono>
#include <ranges>

using namespace exe;
using namespace std::chrono_literals;

//////////////////////////////////////////////////////////////////////

std::error_code TimeoutError() {
  return std::make_error_code(std::errc::timed_out);
}

//////////////////////////////////////////////////////////////////////

void StressTestContract() {
  course::test::TimeBudget budget;
  int iter = 0;

  while (budget) {
    auto [f, p] = future::Contract<int>();

    twist::ed::std::thread producer([p = std::move(p), iter]() mutable {
      std::move(p).Set((int)iter);
    });

    twist::ed::std::thread consumer([f = std::move(f), iter]() mutable {
      int v = future::Get(std::move(f));
      ASSERT_EQ(v, iter);
    });

    producer.join();
    consumer.join();

    ++iter;
  }

  fmt::println("Iterations: {}", iter);
}

//////////////////////////////////////////////////////////////////////

void StressTestPipeline() {
  runtime::MultiThread mt{4};
  mt.Start();

  size_t iter = 0;

  course::test::TimeBudget budget;

  while (budget) {
    size_t pipelines = 1 + (iter++) % 3;

    std::atomic_size_t counter1 = 0;
    std::atomic_size_t counter2 = 0;
    std::atomic_size_t counter3 = 0;

    auto futs = std::views::iota(0ul, pipelines)
                | std::views::transform([&](size_t) {
                    return future::Spawn(mt, [&] {
                        ++counter1;
                        return unit;
                      })
                    | future::Map([&](Unit) {
                        ++counter2;
                        return unit;
                      })
                    | future::Map([&](Unit) {
                        ++counter3;
                        return unit;
                      });
                  })
                | std::ranges::to<std::vector>();

    for (auto&& f : futs) {
      future::Get(std::move(f));
    }

    ASSERT_EQ(counter1.load(), pipelines);
    ASSERT_EQ(counter2.load(), pipelines);
    ASSERT_EQ(counter3.load(), pipelines);

    ++iter;
  }

  fmt::println("Iterations: {}", iter);

  mt.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestFirst() {
  runtime::MultiThread mt{4};
  mt.Start();

  size_t iter = 0;

  course::test::TimeBudget budget;

  while (budget) {
    std::atomic<size_t> done{0};

    auto f = future::Spawn(mt, [&] {
      ++done;
      return 1;
    });

    auto g = future::Spawn(mt, [&] {
      ++done;
      return 2;
    });

    auto first = future::First(std::move(f), std::move(g));

    auto v = future::Get(std::move(first));

    ASSERT_TRUE(v == 1 || v == 2);

    // Barrier
    while (done.load() != 2) {
      twist::ed::std::this_thread::yield();
    }

    ++iter;
  }

  fmt::println("Iterations: {}", iter);

  mt.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestFirstOk() {
  runtime::MultiThread mt{4};
  mt.Start();

  size_t iter = 0;

  course::test::TimeBudget budget;

  while (budget) {
    size_t i = iter;

    std::atomic<size_t> done{0};

    auto f = future::Spawn(mt, [&, i] -> Result<int, std::error_code> {
      Result<int, std::error_code> result;
      if (i % 3 == 0) {
        result = result::Err<int>(TimeoutError());
      } else {
        result = result::Ok<int, std::error_code>(1);
      }
      ++done;
      return result;
    });

    auto g = future::Spawn(mt, [&, i] -> Result<int, std::error_code> {
      Result<int, std::error_code> result;
      if (i % 4 == 0) {
        result = result::Err<int>(TimeoutError());
      } else {
        result = result::Ok<int, std::error_code>(2);
      }
      ++done;
      return result;
    });

    auto first = future::FirstOk(std::move(f), std::move(g));

    auto r = future::Get(std::move(first));

    if (i % 12 != 0) {
      ASSERT_TRUE(r);
      ASSERT_TRUE((*r == 1) || (*r == 2));
    } else {
      ASSERT_FALSE(r);
    }

    // Barrier
    while (done.load() != 2) {
      twist::ed::std::this_thread::yield();
    }

    ++iter;
  }

  fmt::println("Iterations: {}", iter);

  mt.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestBoth() {
  runtime::MultiThread mt{4};
  mt.Start();

  size_t iter = 0;

  course::test::TimeBudget budget;

  while (budget) {
    auto f = future::Spawn(mt, [] {
      return 1;
    });

    auto g = future::Spawn(mt, [] {
      return 2;
    });

    auto both = future::Both(std::move(f), std::move(g));

    auto [x, y] = future::Get(std::move(both));

    ASSERT_EQ(x, 1);
    ASSERT_EQ(y, 2);

    ++iter;
  }

  fmt::println("Iterations: {}", iter);

  mt.Stop();
}

//////////////////////////////////////////////////////////////////////

void StressTestBothOk() {
  runtime::MultiThread mt{4};
  mt.Start();

  size_t iter = 0;

  course::test::TimeBudget budget;

  while (budget) {
    size_t i = iter;

    auto f = future::Spawn(mt, [i] -> Result<int, std::error_code> {
      if (i % 7 == 5) {
        return result::Err<int>(TimeoutError());
      } else {
        return result::Ok<int, std::error_code>(1);
      }
    });

    auto g = future::Spawn(mt, [i]() -> Result<int, std::error_code> {
      if (i % 7 == 6) {
        return result::Err<int>(TimeoutError());
      } else {
        return result::Ok<int, std::error_code>(2);
      }
    });

    auto both = future::BothOk(std::move(f), std::move(g));

    auto r = future::Get(std::move(both));

    if (i % 7 < 5) {
      auto [x, y] = *r;
      ASSERT_EQ(x, 1);
      ASSERT_EQ(y, 2);
    } else {
      ASSERT_FALSE(r);
    }

    ++iter;
  }

  fmt::println("Iterations: {}", iter);

  mt.Stop();
}

//////////////////////////////////////////////////////////////////////

TEST_SUITE(StressFutures) {
  TWIST_STRESS_TEST(Contract, 5s) {
    StressTestContract();
  }

  TWIST_STRESS_TEST(Pipeline, 5s) {
    StressTestPipeline();
  }

  TWIST_STRESS_TEST(First, 5s) {
    StressTestFirst();
  }

  TWIST_STRESS_TEST(FirstOk, 5s) {
    StressTestFirstOk();
  }

  TWIST_STRESS_TEST(Both, 5s) {
    StressTestBoth();
  }

  TWIST_STRESS_TEST(BothOk, 5s) {
    StressTestBothOk();
  }
}

RUN_ALL_TESTS()
