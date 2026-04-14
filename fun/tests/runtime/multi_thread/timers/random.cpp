#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/set_timer.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>

#include <twist/assist/preempt.hpp>
#include <twist/assist/random.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(MultiThreadTimers) {
  TWIST_RANDOMIZE(StartStop, 5s) {
    runtime::MultiThread mt{4};
    mt.WithTimers().Start();
    twist::assist::PreemptionPoint();
    mt.Stop();
  }

  TWIST_RANDOMIZE(SetTimers, 5s) {
    twist::ed::std::random_device rd;
    twist::assist::Choice choice{rd};

    size_t workers = choice(2, 5);

    runtime::MultiThread mt{workers};
    mt.WithTimers();
    mt.Start();

    {
      thread::WaitGroup wg;

      size_t timers = choice(1, 10);

      wg.Add(timers);
      for (size_t i = 1; i <= timers; ++i) {
        auto delay = i * 100us;
        runtime::SetTimer(mt, delay, [&] {
          wg.Done();
        });
      }

      wg.Wait();
    }

    mt.Stop();
  }
}

RUN_ALL_TESTS()
