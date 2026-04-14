#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/mutex.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>

#include <twist/assist/random.hpp>

#include <twist/test/body/plate.hpp>
#include <twist/test/body/either.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeMutex) {
  TWIST_RANDOMIZE(Lock, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t fibers = choice(1, 6);
    size_t writes = choice(1, 5);


    runtime::MultiThread rt{3};
    rt.Start();

    {
      thread::WaitGroup test;

      fiber::Mutex mutex;
      twist::test::body::Plate plate;

      for (size_t i = 0; i < fibers; ++i) {
        test.Add(1);

        fiber::Go(rt, [&] {
          for (size_t j = 0; j < writes; ++j) {
            mutex.Lock();
            plate.Access();
            mutex.Unlock();
          }

          test.Done();
        });
      }

      test.Wait();
    }

    rt.Stop();
  }

  TWIST_RANDOMIZE(LockAndTryLock, 5s) {
    twist::ed::std::random_device rd{};
    twist::assist::Choice choice{rd};

    size_t fibers = choice(1, 6);
    size_t writes = choice(1, 5);


    runtime::MultiThread rt{3};
    rt.Start();

    {
      thread::WaitGroup test;

      fiber::Mutex mutex;
      twist::test::body::Plate plate;

      for (size_t i = 0; i < fibers; ++i) {
        test.Add(1);

        fiber::Go(rt, [&] {
          for (size_t j = 0; j < writes; ++j) {
            if (twist::test::body::Either()) {
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

          test.Done();
        });
      }

      test.Wait();
    }

    rt.Stop();
  }
}

RUN_ALL_TESTS();
