#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/test/body/plate.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(StressMutex) {
  TWIST_STRESS_TEST(StackOverflowSingleFiber, 3s) {
    runtime::MultiThread mt{3};
    mt.Start();

    thread::WaitGroup wg;

    fiber::Mutex mutex;
    twist::test::body::Plate plate;

    wg.Add(1);

    fiber::Go(mt, [&] {
      course::test::TimeBudget time_budget;

      while (time_budget) {
        mutex.Lock();
        plate.Access();
        mutex.Unlock();
      }

      wg.Done();
    });

    wg.Wait();

    fmt::println("# cs = {}", plate.AccessCount());

    mt.Stop();
  }

  TWIST_STRESS_TEST(StackOverflowManyFibers, 10s) {
    runtime::MultiThread mt{3};
    mt.Start();

    thread::WaitGroup wg;

    fiber::Mutex mutex;
    twist::test::body::Plate plate;

    for (size_t i = 0; i < 4096; ++i) {
      wg.Add(1);

      fiber::Go(mt, [&] {
        course::test::TimeBudget time_budget{5s};

        while (time_budget) {
          mutex.Lock();
          plate.Access();
          mutex.Unlock();
        }

        wg.Done();
      });
    }

    wg.Wait();

    fmt::println("# cs = {}", plate.AccessCount());

    mt.Stop();
  }
}

RUN_ALL_TESTS();
