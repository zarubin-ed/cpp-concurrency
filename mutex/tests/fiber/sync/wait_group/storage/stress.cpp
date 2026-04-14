#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/wait_group.hpp>

#include <exe/thread/wait_group.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(WaitGroup) {
  TWIST_STRESS_TEST(Storage, 5s) {
    runtime::MultiThread rt{3};
    rt.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      thread::WaitGroup iter;

      iter.Add(1);

      fiber::Go(rt, [&iter] {
        auto* wg = new fiber::WaitGroup{};

        wg->Add(1);
        fiber::Go([wg] {
          wg->Done();
        });

        wg->Wait();
        delete wg;

        iter.Done();
      });

      iter.Wait();
    }

    rt.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
