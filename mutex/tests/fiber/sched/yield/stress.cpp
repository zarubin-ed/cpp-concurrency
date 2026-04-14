#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(StressFibers) {
  TWIST_STRESS_TEST(Yield, 5s) {
    runtime::MultiThread rt{4};
    rt.Start();

    const size_t kFibers = 17;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(rt, [&] {
        course::test::TimeBudget time_budget;
        while (time_budget) {
          fiber::Yield();
        }

        wg.Done();
      });
    }

    wg.Wait();

    rt.Stop();
  }
}

RUN_ALL_TESTS()
