#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/ed/std/atomic.hpp>
#include <twist/test/body/assert.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressWaitGroup) {
  TWIST_STRESS_TEST(Load, 5s) {
    runtime::MultiThread rt{3};
    rt.Start();

    thread::WaitGroup test_iter;

    test_iter.Add(1);

    fiber::Go(rt, [&] {
      course::test::TimeBudget time_budget;

      fiber::WaitGroup wg;

      size_t iter_count = 0;

      while (time_budget) {
        ++iter_count;

        size_t fibers = 3 + iter_count % 5;

        twist::ed::std::atomic_size_t work = 0;

        for (size_t i = 0; i < fibers; ++i) {
          wg.Add(1);

          fiber::Go([&] {
            for (size_t j = 0; j < 7; ++j) {
              fiber::Yield();
            }
            work.fetch_add(1);
            wg.Done();
          });
        }

        wg.Wait();

        TWIST_TEST_ASSERT(work.load() == fibers, "Missing work");
      }

      fmt::println("# groups = {}", iter_count);

      test_iter.Done();
    });

    test_iter.Wait();

    rt.Stop();
  }
}

RUN_ALL_TESTS();
