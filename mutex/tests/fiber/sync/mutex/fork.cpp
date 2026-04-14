#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>

#include <twist/ed/std/thread.hpp>
#include <twist/ed/std/chrono.hpp>

#include <twist/test/body/assert.hpp>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(Mutex) {

  // Thread

  auto Now() {
    return twist::ed::std::chrono::steady_clock::now();
  }

  void SleepFor(std::chrono::milliseconds delay) {
    twist::ed::std::this_thread::sleep_for(delay);
  }

  TWIST_SIMULATION(UnlockFork, "Fair") {
    runtime::MultiThread mt{4};
    mt.Start();

    fiber::Mutex mutex;

    auto start = Now();

    thread::WaitGroup test;
    test.Add(4);

    fiber::Go(mt, [&] {
      mutex.Lock();
      SleepFor(1s);
      mutex.Unlock();

      test.Done();
    });

    SleepFor(128ms);

    for (size_t i = 0; i < 3; ++i) {
      fiber::Go(mt, [&] {
        mutex.Lock();
        // Sequential
        mutex.Unlock();

        // Parallel
        SleepFor(1s);

        test.Done();
      });
    }

    test.Wait();

    auto elapsed = Now() - start;
    TWIST_TEST_ASSERT(elapsed < 3s, "Fork in Mutex::Unlock");

    mt.Stop();
  }
}

RUN_ALL_TESTS()
