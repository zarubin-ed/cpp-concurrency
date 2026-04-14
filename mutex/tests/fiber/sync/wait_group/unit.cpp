#include <exe/runtime/sandbox.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/wait_group.hpp>

// Testing

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(WaitGroup) {
  SIMPLE_TEST(OneWaiter) {
    runtime::Sandbox sandbox;

    fiber::WaitGroup wg;
    size_t work = 0;
    bool ok = false;

    const size_t kWorkers = 3;

    for (size_t i = 0; i < kWorkers; ++i) {
      wg.Add(1);
      fiber::Go(sandbox, [&] {
        ++work;
        wg.Done();
      });
    }

    fiber::Go(sandbox, [&] {
      wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ok = true;
    });

    sandbox.RunTasks();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(MultipleWaiters) {
    runtime::Sandbox sandbox;

    fiber::WaitGroup wg;

    size_t work = 0;
    size_t acks = 0;

    const size_t kWorkers = 3;
    const size_t kWaiters = 4;

    for (size_t i = 0; i < kWorkers; ++i) {
      wg.Add(1);
      fiber::Go(sandbox, [&] {
        ++work;
        wg.Done();
      });
    }

    for (size_t i = 0; i < kWaiters; ++i) {
      fiber::Go(sandbox, [&] {
        wg.Wait();
        ASSERT_EQ(work, kWorkers);
        ++acks;
      });
    }

    sandbox.RunTasks();

    ASSERT_EQ(acks, kWaiters);
  }

  SIMPLE_TEST(SuspendFiber) {
    runtime::Sandbox sandbox;

    fiber::WaitGroup wg;
    size_t work = 0;
    bool ok = false;

    const size_t kWorkers = 3;

    wg.Add(kWorkers);

    fiber::Go(sandbox, [&] {
      wg.Wait();
      ASSERT_EQ(work, kWorkers);
      ok = true;
    });

    {
      size_t tasks = sandbox.RunTasks();
      ASSERT_LE(tasks, 7);
    }

    for (size_t i = 0; i < kWorkers; ++i) {
      fiber::Go(sandbox, [&] {
        ++work;
        wg.Done();
      });
    }

    sandbox.RunTasks();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(Cyclic) {
    runtime::Sandbox sandbox;

    fiber::WaitGroup wg;

    const size_t kIters = 3;

    for (size_t k = 0; k < kIters; ++k) {
      const size_t kWork = 5;

      size_t work = 0;

      for (size_t i = 0; i < kWork; ++i) {
        wg.Add(1);
        fiber::Go(sandbox, [&] {
          ++work;
          wg.Done();
        });

        fiber::Go(sandbox, [&] {
          wg.Wait();
          ASSERT_EQ(work, kWork);
        });
      }

      sandbox.RunTasks();
    }
  }
}

RUN_ALL_TESTS()
