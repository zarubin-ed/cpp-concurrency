#include <exe/runtime/sandbox.hpp>

#include <exe/fiber/sched/go.hpp>
#include <exe/fiber/sched/yield.hpp>
#include <exe/fiber/sched/sleep_for.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(Fibers) {
  SIMPLE_TEST(Go) {
    runtime::Sandbox sandbox;

    fiber::Go(sandbox, []{});

    size_t tasks = sandbox.RunTasks();
    ASSERT_EQ(tasks, 1);
  }

  SIMPLE_TEST(GoGroup) {
    runtime::Sandbox sandbox;

    const size_t kFibers = 7;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(sandbox, []{});
    }

    size_t tasks = sandbox.RunTasks();
    ASSERT_EQ(tasks, kFibers);
  }

  SIMPLE_TEST(GoChild) {
    runtime::Sandbox sandbox;

    bool flag = false;

    fiber::Go(sandbox, [&] {
      fiber::Go([&] {
        flag = true;
      });
    });

    ASSERT_TRUE(sandbox.RunNextTask());
    ASSERT_FALSE(flag);
    ASSERT_EQ(sandbox.RunTasks(), 1);
    ASSERT_TRUE(flag);
  }

  SIMPLE_TEST(Yield) {
    runtime::Sandbox sandbox;

    fiber::Go(sandbox, [] {
      fiber::Yield();
    });

    ASSERT_EQ(sandbox.RunTasks(), 2);
  }

  SIMPLE_TEST(PingPong) {
    runtime::Sandbox sandbox;

    int turn = 0;

    fiber::Go(sandbox, [&] {
      for (size_t i = 0; i < 3; ++i) {
        ASSERT_EQ(turn, 0);
        turn ^= 1;
        fiber::Yield();
      }
    });

    fiber::Go(sandbox, [&] {
      for (size_t j = 0; j < 3; ++j) {
        ASSERT_EQ(turn, 1);
        turn ^= 1;
        fiber::Yield();
      }
    });

    sandbox.RunTasks();
  }

  SIMPLE_TEST(YieldGroup) {
    runtime::Sandbox sandbox;

    const size_t kFibers = 3;
    const size_t kYields = 4;

    for (size_t i = 0; i < kFibers; ++i) {
      fiber::Go(sandbox, [] {
        for (size_t k = 0; k < kYields; ++k) {
          fiber::Yield();
        }
      });
    }

    size_t tasks = sandbox.RunTasks();
    ASSERT_EQ(tasks, kFibers * (kYields + 1));
  }
}

RUN_ALL_TESTS()
