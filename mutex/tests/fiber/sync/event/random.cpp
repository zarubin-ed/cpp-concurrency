#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

#include <exe/thread/wait_group.hpp>

// Testing

#include <course/test/twist.hpp>

#include <twist/assist/shared.hpp>
#include <twist/test/body/assert.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeEvent) {
  TWIST_RANDOMIZE(MessagePassing, 10s) {
    runtime::MultiThread rt{3};
    rt.Start();

    {
      thread::WaitGroup test;

      twist::assist::Shared<int> data = 0;
      fiber::Event event;

      test.Add(1);

      fiber::Go(rt, [&] {
        data.Write(1);
        event.Fire();

        test.Done();
      });

      test.Add(1);

      fiber::Go(rt, [&] {
        event.Wait();

        int d = data.Read();
        TWIST_TEST_ASSERT(d == 1, "Unfinished work");

        test.Done();
      });

      test.Wait();
    }

    rt.Stop();
  }
}

RUN_ALL_TESTS();
