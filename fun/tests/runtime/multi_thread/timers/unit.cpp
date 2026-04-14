#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/submit_task.hpp>
#include <exe/runtime/set_timer.hpp>
#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

#include <course/test/cpu.hpp>

#include <wheels/core/stop_watch.hpp>

#include <atomic>
#include <thread>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

TEST_SUITE(MultiThreadTimers) {
  SIMPLE_TEST(SetTimer) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    wheels::StopWatch wall;
    course::test::ThreadCPUTimer cpu;

    {
      thread::WaitGroup wg;

      wg.Add(1);
      runtime::SetTimer(mt, 3s, [&wg] {
        wg.Done();
      });

      wg.Wait();
    }

    ASSERT_TRUE(wall.Elapsed() >= 3s);
    ASSERT_TRUE(wall.Elapsed() < 3s + 333ms);

    mt.Stop();
  }

  SIMPLE_TEST(DoNotBurnCpu) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    course::test::ThreadCPUTimer cpu;

    {
      thread::WaitGroup wg;

      wg.Add(1);
      runtime::SetTimer(mt, 3s, [&wg] {
        wg.Done();
      });

      wg.Wait();
    }

    ASSERT_TRUE(cpu.Spent() < 100ms);

    mt.Stop();
  }

  SIMPLE_TEST(SetTimer2) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    for (size_t d = 1; d < 9; ++d) {
      auto delay = 100ms * d;

      wg.Add(1);

      wheels::StopWatch sw;

      runtime::SetTimer(mt, delay, [&wg] {
        wg.Done();
      });

      wg.Wait();

      ASSERT_TRUE(sw.Elapsed() >= delay);
    }

    mt.Stop();
  }

  SIMPLE_TEST(SetTimerTwice) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    for (size_t i = 0; i < 2; ++i) {
      wheels::StopWatch sw;

      {
        wg.Add(1);
        runtime::SetTimer(mt, 1s, [&wg] {
          wg.Done();
        });
        wg.Wait();
      }

      ASSERT_TRUE(sw.Elapsed() >= 1s);
    }

    mt.Stop();
  }

  SIMPLE_TEST(RunTimerInPool) {
    runtime::MultiThread mt{1};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    std::thread::id worker;

    // Get worker id

    wg.Add(1);
    runtime::SubmitTask(mt, [&] {
      worker = std::this_thread::get_id();
      wg.Done();
    });

    wg.Wait();

    // Run timer

    wg.Add(1);
    runtime::SetTimer(mt, 1s, [&] {
      auto tid = std::this_thread::get_id();
      ASSERT_EQ(tid, worker);
      wg.Done();
    });

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(Bubbles) {
    static const size_t kWorkers = 11;

    runtime::MultiThread mt{kWorkers};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    wg.Add(kWorkers);
    for (size_t i = 0; i < kWorkers; ++i) {
      runtime::SetTimer(mt, 1s, [&wg] {
        std::this_thread::sleep_for(1s);
        wg.Done();
      });
    }

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(MultiTimers) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    static const size_t kTimers = 11;

    wg.Add(kTimers);
    for (size_t k = 0; k < kTimers; ++k) {
      runtime::SetTimer(mt, 1s, [&wg] {
        wg.Done();
      });
    }

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(SameDeadline) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    static const size_t kTimers = 5;

    wg.Add(kTimers);
    for (size_t i = 0; i < kTimers; ++i) {
      runtime::SetTimer(mt, 2s, [&wg] {
        wg.Done();
      });
    }

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(Loop) {
    runtime::MultiThread mt{4};
    mt.WithTimers();
    mt.Start();

    thread::WaitGroup wg;

    for (size_t d = 1; d < 10; ++d) {
      wg.Add(5);
      for (size_t k = 0; k < 5; ++k) {
        runtime::SetTimer(mt, 10ms * d, [&wg] {
          wg.Done();
        });
      }
      wg.Wait();
    }

    mt.Stop();
  }
}

RUN_ALL_TESTS()
