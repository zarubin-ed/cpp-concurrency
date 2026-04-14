#include <exe/runtime/sandbox.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

// Testing

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Event) {
  SIMPLE_TEST(OneWaiter) {
    runtime::Sandbox sandbox;

    static const std::string kMessage = "Hello";

    fiber::Event event;
    std::string data;
    bool ok = false;

    fiber::Go(sandbox, [&] {
      event.Wait();
      ASSERT_EQ(data, kMessage);
      ok = true;
    });

    fiber::Go(sandbox, [&] {
      data = kMessage;
      event.Fire();
    });

    sandbox.RunTasks();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(MultipleWaiters) {
    runtime::Sandbox sandbox;

    fiber::Event event;
    int work = 0;
    size_t waiters = 0;

    static const size_t kWaiters = 7;

    for (size_t i = 0; i < kWaiters; ++i) {
      fiber::Go(sandbox, [&] {
        event.Wait();
        ASSERT_EQ(work, 1);
        ++waiters;
      });
    }

    fiber::Go(sandbox, [&] {
      ++work;
      event.Fire();
    });

    sandbox.RunTasks();

    ASSERT_EQ(waiters, kWaiters);
  }

  SIMPLE_TEST(SuspendFiber) {
    runtime::Sandbox sandbox;

    fiber::Event event;
    bool ok = false;

    fiber::Go(sandbox, [&] {
      event.Wait();
      ok = true;
    });

    {
      size_t tasks = sandbox.RunTasks();
      ASSERT_LE(tasks, 7);
    }

    fiber::Go(sandbox, [&] {
      event.Fire();
    });

    sandbox.RunTasks();

    ASSERT_TRUE(ok);
  }
}

RUN_ALL_TESTS()
