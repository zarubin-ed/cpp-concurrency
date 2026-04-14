#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/assist/shared.hpp>
#include <twist/test/body/assert.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressEvent) {
  TWIST_STRESS_TEST(MessagePassing, 5s) {
    runtime::MultiThread rt{3};
    rt.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      thread::WaitGroup test_iter;

      twist::assist::Shared<int> data = 0;
      fiber::Event event;

      test_iter.Add(2);

      fiber::Go(rt, [&] {
        data.Write(1);
        event.Fire();

        test_iter.Done();
      });

      fiber::Go(rt, [&] {
        event.Wait();

        int d = data.Read();
        TWIST_TEST_ASSERT(d == 1, "Missing message");

        test_iter.Done();
      });

      test_iter.Wait();
    }

    rt.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
