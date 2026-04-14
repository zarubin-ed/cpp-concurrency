#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/set_timer.hpp>

#include <exe/thread/wait_group.hpp>

#include <course/test/twist.hpp>
#include <course/test/time_budget.hpp>

#include <random>

using namespace exe;  // NOLINT

////

class Sleeper
    : public std::enable_shared_from_this<Sleeper> {

 public:
  explicit Sleeper(runtime::View rt, thread::WaitGroup& wg)
      : rt_(rt), wg_(&wg) {
  }

  void Start(size_t s) {
    wg_->Add(1);
    twister_.seed(s);

    Sleep();
  }

  void Sleep() {
    auto delay = 1us * (twister_() % 257);

    auto self = shared_from_this();
    runtime::SetTimer(rt_, delay, [self, this] {
      RunTimer();
    });
  }

  void RunTimer() {
    if (budget_) {
      Sleep();
    } else {
      wg_->Done();
    }
  }

 private:
  runtime::View rt_;
  thread::WaitGroup* wg_;
  course::test::TimeBudget budget_;
  std::mt19937_64 twister_;
};

////

TEST_SUITE(MultiThreadTimers) {
  TWIST_STRESS_TEST(Sleepers, 5s) {
    runtime::MultiThread mt{4};
    mt.WithTimers()
        .Start();

    static const size_t kSleepers = 100;

    thread::WaitGroup wg;

    for (size_t s = 0; s < kSleepers; ++s) {
      std::make_shared<Sleeper>(mt, wg)->Start(s);
    }

    wg.Wait();

    mt.Stop();
  }
}

RUN_ALL_TESTS()
