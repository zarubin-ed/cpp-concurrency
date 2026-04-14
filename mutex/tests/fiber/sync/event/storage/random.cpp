#include <exe/runtime/multi_thread.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sync/event.hpp>

// Testing

#include <course/test/twist.hpp>

#include <exe/thread/wait_group.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(RandomizeEvent) {
  TWIST_RANDOMIZE(Storage, 5s) {
    runtime::MultiThread rt{3};
    rt.Start();

    {
      thread::WaitGroup test;

      test.Add(1);

      fiber::Go(rt, [&test] {
        auto* event = new fiber::Event{};

        fiber::Go([event] {
          event->Fire();
        });

        event->Wait();
        delete event;

        test.Done();
      });

      test.Wait();
    }

    rt.Stop();
  }
}

RUN_ALL_TESTS();
