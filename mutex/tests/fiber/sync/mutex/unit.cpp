#include <exe/runtime/sandbox.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sync/event.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <mutex>

// Testing

#include <wheels/test/framework.hpp>

#include <fmt/core.h>

using namespace exe;  // NOLINT

TEST_SUITE(Mutex) {
  SIMPLE_TEST(Lock) {
    runtime::Sandbox sandbox;

    fiber::Mutex mutex;
    size_t cs = 0;

    fiber::Go(sandbox, [&] {
      mutex.Lock();
      ++cs;
      mutex.Unlock();

      mutex.Lock();
      ++cs;
      mutex.Unlock();
    });

    sandbox.RunTasks();

    ASSERT_EQ(cs, 2);
  }

  SIMPLE_TEST(TryLock) {
    runtime::Sandbox sandbox;

    fiber::Go(sandbox, [&] {
      fiber::Mutex mutex;

      {
        ASSERT_TRUE(mutex.TryLock());
        mutex.Unlock();
      }

      {
        mutex.Lock();
        mutex.Unlock();
      }

      ASSERT_TRUE(mutex.TryLock());

      bool join = false;

      fiber::Go([&] {
        ASSERT_FALSE(mutex.TryLock());
        join = true;
      });

      while (!join) {
        fiber::Yield();
      }

      mutex.Unlock();
    });

    sandbox.RunTasks();
  }

  SIMPLE_TEST(Lockable) {
    runtime::Sandbox sandbox;

    fiber::Go(sandbox, [&] {
      fiber::Mutex mutex;

      {
        std::lock_guard guard{mutex};
      }

      {
        std::unique_lock lock{mutex, std::try_to_lock};
        ASSERT_TRUE(lock.owns_lock());
      }
    });

    sandbox.RunTasks();
  }

  SIMPLE_TEST(LockManyTimes) {
    runtime::Sandbox sandbox;

    fiber::Mutex mutex;
    size_t cs = 0;

    fiber::Go(sandbox, [&] {
      for (size_t j = 0; j < 11; ++j) {
        std::lock_guard guard(mutex);
        ++cs;
      }
    });

    sandbox.RunTasks();

    ASSERT_EQ(cs, 11);
  }

  SIMPLE_TEST(Counter) {
    runtime::Sandbox sandbox;

    fiber::Mutex mutex;
    size_t cs = 0;

    static const size_t kFibers = 5;
    static const size_t kSectionsPerFiber = 5;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(sandbox, [&] {
        for (size_t j = 0; j < kSectionsPerFiber; ++j) {
          std::lock_guard guard(mutex);

          ++cs;
          fiber::Yield();
        }
      });
    }

    sandbox.RunTasks();

    fmt::println("# cs = {}, expected = {}",
                 cs, kFibers * kSectionsPerFiber);

    ASSERT_EQ(cs, kFibers * kSectionsPerFiber);
  }

  SIMPLE_TEST(SuspendFiber) {
    runtime::Sandbox sandbox;

    fiber::Mutex mutex;
    fiber::Event unlock;

    fiber::Go(sandbox, [&] {
      mutex.Lock();
      unlock.Wait();
      mutex.Unlock();
    });

    bool cs = false;

    fiber::Go(sandbox, [&] {
      mutex.Lock();
      cs = true;
      mutex.Unlock();
    });

    {
      size_t tasks = sandbox.RunTasks();
      ASSERT_LE(tasks, 17);
      ASSERT_FALSE(cs);
    }

    fiber::Go(sandbox, [&] {
      unlock.Fire();
    });

    sandbox.RunTasks();

    ASSERT_TRUE(cs);
  }

  SIMPLE_TEST(Fifo) {
    runtime::Sandbox sandbox;

    fiber::Mutex mutex;

    fiber::Go(sandbox, [&] {
      mutex.Lock();

      for (size_t i = 0; i < 1024; ++i) {
        fiber::Yield();
      }

      mutex.Unlock();
    });

    const size_t kWaiters = 16;

    sandbox.RunAtMostTasks(7);  // Lock mutex

    size_t next_waiter = 0;

    for (size_t i = 0; i < kWaiters; ++i) {
      fiber::Go(sandbox, [&, i] {
        mutex.Lock();

        ASSERT_EQ(next_waiter, i);
        ++next_waiter;

        mutex.Unlock();
      });

      // mutex.Lock() -> wait queue
      sandbox.RunAtMostTasks(7);
    }

    sandbox.RunTasks();
  }
}

RUN_ALL_TESTS()
