#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/submit_task.hpp>
#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>

#include <twist/ed/std/atomic.hpp>

#include <twist/test/body/assert.hpp>

#include <twist/assist/preempt.hpp>
#include <twist/assist/random.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeMultiThreadTasks) {
  TWIST_RANDOMIZE(StartStop, 5s) {
    runtime::MultiThread mt{4};
    mt.Start();
    twist::assist::PreemptionPoint();
    mt.Stop();
  }

  TWIST_RANDOMIZE(SubmitAndWait, 5s) {
    twist::ed::std::random_device rd;
    twist::assist::Choice choice{rd};

    size_t workers = choice(2, 5);

    runtime::MultiThread mt{workers};
    mt.Start();

    {
      thread::WaitGroup wg;

      size_t tasks = choice(1, 8);

      for (size_t i = 0; i < tasks; ++i) {
        wg.Add(1);

        runtime::SubmitTask(mt, [&wg] {
          wg.Done();
        });
      }

      wg.Wait();
    }

    mt.Stop();
  }

  TWIST_RANDOMIZE(Concurrent, 5s) {
    twist::ed::std::random_device rd;
    twist::assist::Choice choice{rd};

    size_t workers = choice(2, 5);

    runtime::MultiThread mt{workers};
    mt.Start();

    {
      thread::WaitGroup wg;

      size_t todo = choice(1, 5);
      twist::ed::std::atomic_size_t done{0};

      for (size_t i = 0; i < todo; ++i) {
        wg.Add(1);

        runtime::SubmitTask(mt, [&] {
          wg.Add(1);
          runtime::SubmitTask(mt, [&] {
            done.fetch_add(1);
            wg.Done();
          });

          wg.Done();
        });
      }

      wg.Wait();

      TWIST_TEST_ASSERT(done.load() == todo, "Missing tasks");
    }

    mt.Stop();
  }
}

RUN_ALL_TESTS()
