#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <twist/assist/random.hpp>
#include <twist/test/body/plate.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(StressMutex) {
  TWIST_STRESS_TEST(Iter, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    runtime::MultiThread rt{3};
    rt.Start();

    course::test::TimeBudget time_budget;

    size_t iter_count = 0;

    while (time_budget) {
      ++iter_count;

      // Parameters
      size_t fibers = choice(1, 6);
      size_t writes = choice(1, 5);

      thread::WaitGroup test_iter;

      twist::test::body::Plate plate;
      fiber::Mutex mutex;

      for (size_t i = 0; i < fibers; ++i) {
        test_iter.Add(1);

        fiber::Go(rt, [&] {
          for (size_t j = 0; j < writes; ++j) {
            mutex.Lock();
            plate.Access();
            mutex.Unlock();
          }

          test_iter.Done();
        });
      }

      test_iter.Wait();
    }

    rt.Stop();

    fmt::println("# iterations = {}", iter_count);
  }

  TWIST_STRESS_TEST(Load, 5s) {
    // Parameters
    const size_t kFibers = 11;
    const size_t kThreads = 3;

    runtime::MultiThread rt{kThreads};
    rt.Start();

    thread::WaitGroup wg;

    fiber::Mutex mutex;
    twist::test::body::Plate plate;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);

      fiber::Go(rt, [&] {
        course::test::TimeBudget time_budget;

        bool lock = false;

        while (time_budget) {
          lock = !lock;

          if (lock) {
            mutex.Lock();
            plate.Access();
            mutex.Unlock();
          } else {
            if (mutex.TryLock()) {
              plate.Access();
              mutex.Unlock();
            }
          }
        }

        wg.Done();
      });
    }

    wg.Wait();

    rt.Stop();
  }
}

RUN_ALL_TESTS();
