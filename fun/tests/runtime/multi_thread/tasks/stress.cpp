#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/submit_task.hpp>
#include <exe/thread/wait_group.hpp>

#include <twist/ed/std/atomic.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(StressMultiThreadTasks) {
  TWIST_STRESS_TEST(SubmitAndWait, 5s) {
    runtime::MultiThread mt{4};
    mt.Start();

    course::test::TimeBudget time_budget;

    for (size_t iter = 0; time_budget; ++iter) {
      size_t todo = 1 + iter % 11;
      twist::ed::std::atomic_size_t done{0};

      thread::WaitGroup wg;
      for (size_t i = 0; i < todo; ++i) {
        wg.Add(1);
        runtime::SubmitTask(mt, [&] {
          wg.Done();
        });
      }

      wg.Wait();
    }

    mt.Stop();
  }
}

RUN_ALL_TESTS()
