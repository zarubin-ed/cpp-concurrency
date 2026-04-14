#include <exe/runtime/sandbox.hpp>
#include <exe/runtime/submit_task.hpp>

#include <wheels/test/framework.hpp>

using namespace exe;  // NOLINT

TEST_SUITE(SandboxTasks) {
  SIMPLE_TEST(JustWorks) {
    runtime::Sandbox sandbox;

    size_t step = 0;

    ASSERT_TRUE(sandbox.IsEmpty());

    ASSERT_FALSE(sandbox.RunNextTask());
    ASSERT_EQ(sandbox.RunAtMostTasks(99), 0);

    runtime::SubmitTask(sandbox, [&] {
      step = 1;
    });

    ASSERT_FALSE(sandbox.IsEmpty());

    ASSERT_EQ(step, 0);

    runtime::SubmitTask(sandbox, [&] {
      step = 2;
    });

    ASSERT_EQ(step, 0);

    ASSERT_TRUE(sandbox.RunNextTask());

    ASSERT_EQ(step, 1);

    ASSERT_FALSE(sandbox.IsEmpty());

    runtime::SubmitTask(sandbox, [&] {
      step = 3;
    });

    ASSERT_EQ(sandbox.RunAtMostTasks(99), 2);
    ASSERT_EQ(step, 3);

    ASSERT_TRUE(sandbox.IsEmpty());
    ASSERT_FALSE(sandbox.RunNextTask());
  }

  SIMPLE_TEST(Empty) {
    runtime::Sandbox sandbox;

    ASSERT_FALSE(sandbox.RunNextTask());
    ASSERT_EQ(sandbox.RunAtMostTasks(7), 0);
    ASSERT_EQ(sandbox.RunTasks(), 0);
  }

  void Countdown(runtime::Sandbox& sandbox, size_t k) {
    if (k > 0) {
      runtime::SubmitTask(sandbox, [&sandbox, k] {
        Countdown(sandbox, k - 1);
      });
    }
  }

  SIMPLE_TEST(RunAtMost) {
    runtime::Sandbox sandbox;

    Countdown(sandbox, 256);

    size_t tasks = 0;
    do {
      tasks += sandbox.RunAtMostTasks(7);
    } while (!sandbox.IsEmpty());

    ASSERT_EQ(tasks, 256);
  }

  SIMPLE_TEST(RunAtMostNewTasks) {
    runtime::Sandbox sandbox;

    runtime::SubmitTask(sandbox, [&] {
      runtime::SubmitTask(sandbox, [] {});
    });

    ASSERT_EQ(sandbox.RunAtMostTasks(2), 2);
  }

  SIMPLE_TEST(Run) {
    runtime::Sandbox sandbox;

    Countdown(sandbox, 117);

    ASSERT_EQ(sandbox.RunTasks(), 117);
  }

  SIMPLE_TEST(RunTwice) {
    runtime::Sandbox sandbox;

    Countdown(sandbox, 11);

    ASSERT_EQ(sandbox.RunTasks(), 11);

    Countdown(sandbox, 7);

    ASSERT_EQ(sandbox.RunTasks(), 7);
  }
}

RUN_ALL_TESTS()
