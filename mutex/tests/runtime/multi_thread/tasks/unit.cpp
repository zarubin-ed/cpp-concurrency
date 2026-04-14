#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/submit_task.hpp>
#include <exe/thread/wait_group.hpp>

#include <wheels/test/framework.hpp>

// Timers
#include <course/test/cpu.hpp>
#include <wheels/core/stop_watch.hpp>

#include <atomic>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;  // NOLINT

using namespace exe;  // NOLINT

TEST_SUITE(MultiThread) {
  SIMPLE_TEST(WaitTask) {
    runtime::MultiThread mt{4};

    mt.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    runtime::SubmitTask(mt, [&wg] {
      wg.Done();
    });

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(Wait) {
    runtime::MultiThread mt{4};

    mt.Start();

    thread::WaitGroup wg;

    wg.Add(1);

    runtime::SubmitTask(mt, [&wg] {
      std::this_thread::sleep_for(1s);
      wg.Done();
    });

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(MultiWait) {
    runtime::MultiThread mt{1};

    mt.Start();

    for (size_t i = 0; i < 3; ++i) {
      thread::WaitGroup wg;

      wg.Add(1);

      runtime::SubmitTask(mt, [&wg] {
        std::this_thread::sleep_for(1s);
        wg.Done();
      });

      wg.Wait();
    }

    mt.Stop();
  }

  SIMPLE_TEST(ManyTasks) {
    runtime::MultiThread mt{4};

    mt.Start();

    static const size_t kTasks = 17;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kTasks; ++i) {
      wg.Add(1);
      runtime::SubmitTask(mt, [&wg] {
        wg.Done();
      });
    }

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(Parallel) {
    runtime::MultiThread mt{4};

    mt.Start();

    std::atomic<bool> fast{false};

    thread::WaitGroup wg;

    wg.Add(1);
    runtime::SubmitTask(mt, [&] {
      std::this_thread::sleep_for(1s);
      wg.Done();
    });

    wg.Add(1);
    runtime::SubmitTask(mt, [&] {
      fast.store(true);
      wg.Done();
    });

    std::this_thread::sleep_for(100ms);

    ASSERT_EQ(fast.load(), true);

    wg.Wait();
    mt.Stop();
  }

  SIMPLE_TEST(TwoPools) {
    runtime::MultiThread mt1{1};
    runtime::MultiThread mt2{1};

    mt1.Start();
    mt2.Start();

    wheels::StopWatch stop_watch;

    thread::WaitGroup wg1;
    wg1.Add(1);
    runtime::SubmitTask(mt1, [&] {
      std::this_thread::sleep_for(1s);
      wg1.Done();
    });

    thread::WaitGroup wg2;
    wg2.Add(1);
    runtime::SubmitTask(mt2, [&] {
      std::this_thread::sleep_for(1s);
      wg2.Done();
    });

    wg2.Wait();
    mt2.Stop();

    wg1.Wait();
    mt1.Stop();

    ASSERT_TRUE(stop_watch.Elapsed() < 1500ms);
  }

  SIMPLE_TEST(DoNotBurnCPU) {
    runtime::MultiThread mt{4};

    mt.Start();

    thread::WaitGroup wg;

    // Warmup
    for (size_t i = 0; i < 4; ++i) {
      wg.Add(1);
      runtime::SubmitTask(mt, [&wg] {
        std::this_thread::sleep_for(100ms);
        wg.Done();
      });
    }

    course::test::ProcessCPUTimer cpu_timer;

    std::this_thread::sleep_for(1s);

    wg.Wait();
    mt.Stop();

    ASSERT_TRUE(cpu_timer.Spent() < 100ms);
  }

  SIMPLE_TEST(Here1) {
    runtime::MultiThread mt{1};
    mt.Start();

    ASSERT_FALSE(mt.Here());

    thread::WaitGroup wg;
    wg.Add(1);

    runtime::SubmitTask(mt, [&] {
      ASSERT_TRUE(mt.Here());
      wg.Done();
    });

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(Here2) {
    runtime::MultiThread mt1{1};
    runtime::MultiThread mt2{1};

    mt1.Start();
    mt2.Start();

    ASSERT_FALSE(mt1.Here());
    ASSERT_FALSE(mt2.Here());

    thread::WaitGroup wg;
    wg.Add(2);

    runtime::SubmitTask(mt1, [&] {
      ASSERT_TRUE(mt1.Here());
      ASSERT_FALSE(mt2.Here());
      wg.Done();
    });

    runtime::SubmitTask(mt2, [&] {
      ASSERT_FALSE(mt1.Here());
      ASSERT_TRUE(mt2.Here());
      wg.Done();
    });

    wg.Wait();

    mt1.Stop();
    mt2.Stop();
  }

  SIMPLE_TEST(SubmitAfterWait) {
    runtime::MultiThread mt{4};

    mt.Start();

    thread::WaitGroup wg;

    wg.Add(1);
    runtime::SubmitTask(mt, [&] {
      std::this_thread::sleep_for(500ms);

      wg.Add(1);
      runtime::SubmitTask(mt, [&] {
        std::this_thread::sleep_for(500ms);
        wg.Done();
      });

      wg.Done();
    });

    wg.Wait();

    mt.Stop();
  }

  TEST(UseThreads, wheels::test::TestOptions().TimeLimit(1s)) {
    runtime::MultiThread mt{4};
    mt.Start();

    thread::WaitGroup wg;

    for (size_t i = 0; i < 4; ++i) {
      wg.Add(1);
      runtime::SubmitTask(mt, [&wg] {
        std::this_thread::sleep_for(750ms);
        wg.Done();
      });
    }

    wg.Wait();
    mt.Stop();
  }

  TEST(TooManyThreads, wheels::test::TestOptions().TimeLimit(2s)) {
    runtime::MultiThread mt{3};

    mt.Start();

    thread::WaitGroup wg;

    for (size_t i = 0; i < 4; ++i) {
      wg.Add(1);
      runtime::SubmitTask(mt, [&wg] {
        std::this_thread::sleep_for(750ms);
        wg.Done();
      });
    }

    wheels::StopWatch stop_watch;

    wg.Wait();
    mt.Stop();

    ASSERT_TRUE(stop_watch.Elapsed() > 1s);
  }

  SIMPLE_TEST(CrossSubmit) {
    runtime::MultiThread mt1{1};
    runtime::MultiThread mt2{1};

    mt1.Start();
    mt2.Start();

    thread::WaitGroup wg;
    wg.Add(1);

    runtime::SubmitTask(mt1, [&] {
      ASSERT_TRUE(mt1.Here());
      runtime::SubmitTask(mt2, [&] {
        ASSERT_TRUE(mt2.Here());
        wg.Done();
      });
    });

    wg.Wait();

    mt1.Stop();
    mt2.Stop();
  }

  SIMPLE_TEST(TaskLifetime) {
    runtime::MultiThread mt{4};

    mt.Start();

    struct Widget {};

    auto w = std::make_shared<Widget>();

    thread::WaitGroup wg;
    for (int i = 0; i < 4; ++i) {
      wg.Add(1);
      runtime::SubmitTask(mt, [w, &wg] {
        wg.Done();
      });
    }

    std::this_thread::sleep_for(500ms);

    ASSERT_EQ(w.use_count(), 1);

    wg.Wait();

    mt.Stop();
  }

  SIMPLE_TEST(JoinWorkerThreads) {
    runtime::MultiThread mt{1};
    mt.Start();

    bool done = false;

    thread::WaitGroup wg;
    wg.Add(1);

    runtime::SubmitTask(mt, [&] {
      wg.Done();

      {
        // Epilogue
        std::this_thread::sleep_for(1s);
        done = true;
      }
    });

    wg.Wait();

    mt.Stop();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(Racy) {
    runtime::MultiThread mt{4};

    mt.Start();

    std::atomic<size_t> racy_counter{0};

    static const size_t kTasks = 100500;

    thread::WaitGroup wg;

    for (size_t i = 0; i < kTasks; ++i) {
      wg.Add(1);
      runtime::SubmitTask(mt, [&] {
        int old = racy_counter.load();
        racy_counter.store(old + 1);

        wg.Done();
      });
    }

    wg.Wait();

    mt.Stop();

    std::cout << "Racy counter value: " << racy_counter << std::endl;

    ASSERT_LE(racy_counter.load(), kTasks);
  }
}

RUN_ALL_TESTS()
