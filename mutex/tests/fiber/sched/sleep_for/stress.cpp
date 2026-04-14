#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sched/sleep_for.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <random>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(StressFibers) {
  TWIST_STRESS_TEST(Sleepers1, 5s) {
    runtime::MultiThread rt{4};
    rt.WithTimers()
        .Start();

    const size_t kFibers = 17;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(rt, [&, i] {
        course::test::TimeBudget time_budget;

        for (size_t k = 0; time_budget; ++k) {
          switch ((k + i) % 3) {
            case 0:
              fiber::Yield();
              break;
            case 1:
              fiber::SleepFor(0us);
              break;
            case 2:
              fiber::SleepFor(1us);
              break;
          }
        }

        wg.Done();
      });
    }

    wg.Wait();

    rt.Stop();
  }

  TWIST_STRESS_TEST(Sleepers2, 5s) {
    runtime::MultiThread rt{4};
    rt.WithTimers()
        .Start();

    const size_t kFibers = 29;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(rt, [&] {
        course::test::TimeBudget time_budget;

        while (time_budget) {
          fiber::SleepFor(0us);
        }

        wg.Done();
      });
    }

    wg.Wait();

    rt.Stop();
  }

  TWIST_STRESS_TEST(Sleepers3, 5s) {
    runtime::MultiThread rt{4};
    rt.WithTimers()
        .Start();

    const size_t kFibers = 38;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(rt, [&, i] {
        course::test::TimeBudget time_budget;

        std::mt19937_64 twister{i};

        while (time_budget) {
          fiber::SleepFor(1us * (twister() % 257));
        }

        wg.Done();
      });
    }

    wg.Wait();

    rt.Stop();
  }

  TWIST_STRESS_TEST(Sleepers4, 5s) {
    runtime::MultiThread rt{4};
    rt.WithTimers()
        .Start();

    const size_t kFibers = 47;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kFibers; ++i) {
      wg.Add(1);
      fiber::Go(rt, [&, i] {
        course::test::TimeBudget time_budget;

        std::mt19937_64 twister{i};

        while (time_budget) {
          fiber::SleepFor(1us * (twister() % 1007));
        }

        wg.Done();
      });
    }

    wg.Wait();

    rt.Stop();
  }
}

RUN_ALL_TESTS()
