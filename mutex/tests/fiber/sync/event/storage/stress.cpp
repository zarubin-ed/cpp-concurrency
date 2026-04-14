#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressEvent) {
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
        auto* event = new fiber::Event{};

        fiber::Go([event] {
          event->Fire();
        });

        event->Wait();
        delete event;

        iter.Done();
      });

      iter.Wait();
    }

    rt.Stop();

    fmt::println("# iterations = {}", iter_count);
  }
}

RUN_ALL_TESTS();
