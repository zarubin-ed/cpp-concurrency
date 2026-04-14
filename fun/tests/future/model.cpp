#include <exe/future/make/contract.hpp>

#include <exe/future/terminate/get.hpp>

#include <course/test/twist.hpp>

#include <twist/build.hpp>

#include <twist/ed/std/thread.hpp>

#include <twist/assist/shared.hpp>

#include <twist/test/body/assert.hpp>

static_assert(twist::build::kIsolatedSim);

using namespace exe;  // NOLINT

TEST_SUITE(ModelContract) {
  TWIST_MODEL(GetSet, {}) {
    twist::assist::Shared<int> shr{0};

    auto [f, p] = future::Contract<int>();

    twist::ed::std::thread t([&, p = std::move(p)] mutable {
      shr.Write(1);
      std::move(p).Set(1);
    });

    int m = future::Get(std::move(f));
    TWIST_TEST_ASSERT(m == 1, "Unexpected message");
    shr.Read();

    t.join();
  }

  TWIST_MODEL(ConsumeSet, {}) {
    twist::assist::Shared<int> shr{0};

    auto [f, p] = future::Contract<int>();

    bool done = false;

    twist::ed::std::thread t([&, p = std::move(p)] mutable {
      shr.Write(1);
      std::move(p).Set(1);
    });

    std::move(f).Consume([&](int m) {
      TWIST_TEST_ASSERT(m == 1, "Unexpected message");
      shr.Read();
      done = true;
    });

    t.join();

    TWIST_TEST_ASSERT(done, "Missing callback invocation");
  }
}

RUN_ALL_TESTS()
