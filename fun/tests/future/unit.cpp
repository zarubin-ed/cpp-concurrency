#include <exe/runtime/multi_thread.hpp>
#include <exe/runtime/sandbox.hpp>

#include <exe/future/make/contract.hpp>
#include <exe/future/make/value.hpp>
#include <exe/future/make/spawn.hpp>
#include <exe/future/make/run.hpp>
#include <exe/future/make/return.hpp>
#include <exe/future/make/just.hpp>

#include <exe/future/make/result/ok.hpp>
#include <exe/future/make/result/err.hpp>

#include <exe/future/combine/seq/map.hpp>
#include <exe/future/combine/seq/flatten.hpp>
#include <exe/future/combine/seq/flat_map.hpp>
#include <exe/future/combine/seq/via.hpp>
#include <exe/future/combine/seq/after.hpp>

#include <exe/future/combine/seq/result/map_ok.hpp>
#include <exe/future/combine/seq/result/and_then.hpp>
#include <exe/future/combine/seq/result/or_else.hpp>

#include <exe/future/combine/concur/first.hpp>
#include <exe/future/combine/concur/all.hpp>

#include <exe/future/combine/concur/result/first.hpp>
#include <exe/future/combine/concur/result/all.hpp>

#include <exe/future/terminate/get.hpp>
#include <exe/future/terminate/detach.hpp>

#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <course/test/cpu.hpp>

#include <wheels/test/framework.hpp>

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <tuple>

using namespace exe;  // NOLINT

using namespace std::chrono_literals;  // NOLINT

std::error_code TimeoutError() {
  return std::make_error_code(std::errc::timed_out);
}

std::error_code IoError() {
  return std::make_error_code(std::errc::io_error);
}

TEST_SUITE(Futures) {
  SIMPLE_TEST(ContractValue) {
    auto [f, p] = future::Contract<std::string>();

    std::move(p).Set("Hi");
    std::string s = future::Get(std::move(f));

    ASSERT_EQ(s, "Hi");
  }

  SIMPLE_TEST(ContractDetach) {
    auto [f, p] = future::Contract<int>();

    future::Detach(std::move(f));
    std::move(p).Set(1);
  }

  struct MoveOnly {
    MoveOnly() = default;

    // Movable
    MoveOnly(MoveOnly&&) = default;
    MoveOnly& operator=(MoveOnly&&) = default;

    // Non-copyable
    MoveOnly(const MoveOnly&) = delete;
    MoveOnly& operator=(const MoveOnly&) = delete;
  };

  SIMPLE_TEST(ContractMoveOnlyValue) {
    auto [f, p] = future::Contract<MoveOnly>();

    std::move(p).Set(MoveOnly{});
    auto v = future::Get(std::move(f));

    WHEELS_UNUSED(v);
  }

  struct NonDefaultConstructible {
    NonDefaultConstructible(int) {};
  };

  SIMPLE_TEST(ContractNonDefaultConstructibleValue) {
    auto [f, p] = future::Contract<NonDefaultConstructible>();

    std::move(p).Set({128});
    future::Get(std::move(f));
  }

  SIMPLE_TEST(Value) {
    auto f = future::Value(std::string("Hello"));
    auto s = future::Get(std::move(f));

    ASSERT_EQ(s, "Hello");
  }

  SIMPLE_TEST(Return) {
    auto f = future::Return(16);
    auto v = future::Get(std::move(f));

    ASSERT_EQ(v, 16);
  }

  SIMPLE_TEST(Just) {
    auto f = future::Just();
    auto u = future::Get(std::move(f));
    ASSERT_TRUE(u == unit);
  }

  SIMPLE_TEST(SpawnMultiThread) {
    runtime::MultiThread mt{4};
    mt.Start();

    auto compute = future::Spawn(mt, [&mt] -> int {
      ASSERT_TRUE(mt.Here());
      return 11;
    });

    int v = future::Get(std::move(compute));

    ASSERT_EQ(v, 11);

    mt.Stop();
  }

  SIMPLE_TEST(RunMultiThread) {
    runtime::MultiThread mt{4};
    mt.Start();

    auto compute = future::Run(mt, [&mt] -> int {
      ASSERT_TRUE(mt.Here());
      return 11;
    });

    int v = future::Get(std::move(compute));

    ASSERT_EQ(v, 11);

    mt.Stop();
  }

  SIMPLE_TEST(SpawnSandbox) {
    runtime::Sandbox loop;

    bool done = false;

    auto f = future::Spawn(loop, [&] {
      done = true;
      return unit;
    });

    future::Detach(std::move(f));

    ASSERT_FALSE(done);

    loop.RunTasks();

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(SpawnMultiThreadWait) {
    runtime::MultiThread mt{4};
    mt.Start();

    auto f = future::Spawn(mt, [] {
      std::this_thread::sleep_for(1s);
      return 11;
    });

    course::test::ProcessCPUTimer timer;

    auto v = future::Get(std::move(f));

    ASSERT_TRUE(timer.Spent() < 100ms);

    ASSERT_EQ(v, 11);

    mt.Stop();
  }

  SIMPLE_TEST(Map) {
    auto f = future::Value(1)
             | future::Map([](int v) { return v + 1; });

    auto v = future::Get(std::move(f));

    ASSERT_EQ(v, 2);
  }

  SIMPLE_TEST(MapSandbox) {
    runtime::Sandbox loop;

    bool done = false;

    auto f = future::Spawn(loop, [] { return 1; });
    auto g = std::move(f)
             | future::Map([&](int v) {
                 done = true;
                 return v + 2;
               });

    future::Detach(std::move(g));

    size_t tasks = loop.RunTasks();
    ASSERT_LE(tasks, 2);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(MapMultiThread) {
    runtime::MultiThread mt{1};
    mt.Start();

    auto f = future::Spawn(mt, [&mt] {
      ASSERT_TRUE(mt.Here());
      return 1;
    });

    auto g = std::move(f)
             | future::Map([&mt](int v) {
                 ASSERT_TRUE(mt.Here());
                 return v + 2;
               });

    future::Get(std::move(g));

    mt.Stop();
  }

  SIMPLE_TEST(MoveMap) {
    runtime::MultiThread mt{4};
    mt.Start();

    auto f = future::Spawn(mt, [&mt] {
      ASSERT_TRUE(mt.Here());
      return 1;
    });

    auto ff = std::move(f);

    auto g = std::move(ff)
             | future::Map([&mt](int v) {
                 ASSERT_TRUE(mt.Here());
                 return v + 2;
               });

    future::Get(std::move(g));

    mt.Stop();
  }

  SIMPLE_TEST(Via1) {
    runtime::Sandbox loop1;
    runtime::Sandbox loop2;

    size_t steps = 0;

    future::Just()
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([](Unit) {
            return unit;
          })
        | future::Via(loop2)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([&](Unit) {
            ++steps;
            return 1;
          })
        | future::Map([&](int v) {
            ASSERT_EQ(v, 1);
            ++steps;
            return unit;
          })
        | future::Detach();

    ASSERT_LE(loop1.RunTasks(), 2);
    ASSERT_EQ(steps, 1);
    loop2.RunTasks();
    ASSERT_EQ(steps, 3);
    loop1.RunTasks();
    ASSERT_EQ(steps, 6);
  }

  SIMPLE_TEST(AfterMap) {
    runtime::Sandbox sandbox;

    bool done = false;

    auto f = future::Value(2)
             | future::Via(sandbox)
             | future::After(2s)
             | future::Map([&](int v) {
                 done = true;
                 return v + 1;
               });

    future::Detach(std::move(f));

    sandbox.RunTasks();

    ASSERT_FALSE(done);

    ASSERT_EQ(sandbox.AdvanceClockBy(1s), 0);
    ASSERT_EQ(sandbox.AdvanceClockBy(1s), 1);

    ASSERT_FALSE(done);

    ASSERT_GT(sandbox.RunTasks(), 0);

    ASSERT_TRUE(done);
  }

  SIMPLE_TEST(AfterMapMultiThread) {
    runtime::MultiThread mt{4};
    mt.WithTimers().Start();

    auto f = future::Value(2)
             | future::Via(mt)
             | future::After(2s)
             | future::Map([&](int v) {
                 ASSERT_TRUE(mt.Here());
                 return v + 1;
               });

    auto v = future::Get(std::move(f));
    ASSERT_EQ(v, 3);

    mt.Stop();
  }

  SIMPLE_TEST(AfterMapMultiThread2) {
    runtime::MultiThread mt{4};
    mt.WithTimers().Start();

    auto f = future::Value(2)
             | future::Via(mt)
             | future::After(1s);

    std::this_thread::sleep_for(2s);

    auto g = std::move(f)
             | future::Map([&](int v) {
                 ASSERT_TRUE(mt.Here());
                 return v + 1;
               });

    auto v = future::Get(std::move(g));
    ASSERT_EQ(v, 3);

    mt.Stop();
  }

  SIMPLE_TEST(Flatten) {
    runtime::Sandbox loop;

    auto ff = future::Spawn(loop, [&] {
      return future::Spawn(loop, [] {
        return 7;
      });
    });

    auto f = std::move(ff) | future::Flatten();

    bool ok = false;

    std::move(f) | future::Map([&ok](int v) -> Unit {
      ASSERT_EQ(v, 7);
      ok = true;
      return {};
    }) | future::Detach();

    ASSERT_FALSE(ok);

    loop.RunTasks();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(FlatMap) {
    runtime::Sandbox loop;

    auto f = future::Spawn(loop, [] { return 23; })
             | future::FlatMap([&](int v) {
                 return future::Spawn(loop, [v] {
                   return v + 5;
                 });
               })
             | future::Map([](int v) {
                 return v + 1;
               });

    bool ok = true;

    std::move(f)
        | future::Map([&](int v) {
            ASSERT_EQ(v, 29);
            ok = true;
            return unit;
          })
        | future::Detach();

    loop.RunTasks();

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(MoveOnly2) {
    auto f = future::Value(MoveOnly{})
             | future::Map([](MoveOnly v) {
                 return v;
               })
             | future::FlatMap([](MoveOnly v) {
                 return future::Value(std::move(v));
               })
             | future::Map([](MoveOnly v) {
                 return v;
               });

    future::Get(std::move(f));
  }

  struct Widget {
    int x;
  };

  struct Gadget {
    int y;
  };

  SIMPLE_TEST(MapOk) {
    {
      auto f = future::Ok<int, std::error_code>(1)
               | future::MapOk([](int v) {
                   return v + 1;
                 });

      auto r = future::Get(std::move(f));
      ASSERT_TRUE(r);
      ASSERT_EQ(*r, 2);
    }

    {
      auto f = future::Err<int, std::error_code>(TimeoutError())
               | future::MapOk([](int) {
                   FAIL_TEST("Unreachable");
                   return unit;
                 });

      auto r = future::Get(std::move(f));
      ASSERT_FALSE(r);
    }

  }

  SIMPLE_TEST(AndThen) {
    auto f = future::Ok<std::string, std::error_code>(std::string("ok"))
             | future::AndThen([](std::string s) {
                 return future::Ok<std::string, std::error_code>(s + "a");
               })
             | future::AndThen([](std::string s) {
                 return future::Ok<std::string, std::error_code>(s + "y");
               });

    auto r = future::Get(std::move(f));

    ASSERT_TRUE(r);
    ASSERT_EQ(*r, "okay");
  }

  SIMPLE_TEST(OrElse) {
    auto failure = [] {
      return future::Err<std::string, std::error_code>(IoError());
    };

    auto f = failure()
             | future::OrElse([](std::error_code e) {
                 ASSERT_EQ(e, IoError());
                 return future::Ok<std::string, std::error_code>(std::string("fallback"));
               });

    auto r = future::Get(std::move(f));

    ASSERT_TRUE(r);
    ASSERT_EQ(*r, "fallback");
  }

  SIMPLE_TEST(TryPipeline) {
    auto [f, p] = future::Contract<int>();

    auto g = std::move(f)
             | future::Map([](int v) {
                 return v + 1;
               })
             | future::Map([](int v) {
                 return v + 2;
               })
             | future::Map([](int v) {
                 return result::Ok<int, std::error_code>(v);
               })
             | future::OrElse([](std::error_code) {
                 FAIL_TEST("Unreachable");
                 return future::Ok<int, std::error_code>(111);
               })
             | future::AndThen([](int /*v*/) {
                 return future::Err<int, std::error_code>(TimeoutError());
               })
             | future::MapOk([](int v) {
                 FAIL_TEST("Unreachable");
                 return v + 1;
               })
             | future::OrElse([](std::error_code) {
                 return future::Ok<int, std::error_code>(17);
               })
             | future::MapOk([](int v) {
                 return v + 1;
               });

    std::move(p).Set(3);

    auto r = future::Get(std::move(g));

    ASSERT_TRUE(r);
    ASSERT_EQ(*r, 18);
  }

  SIMPLE_TEST(ValueTypes) {
    runtime::MultiThread mt{4};
    mt.Start();

    auto f = future::Value(Widget{1})
             | future::Map([](Widget w) {
                 return Gadget{w.x};
               })
             | future::FlatMap([](Gadget g) {
                 return future::Value(Widget{g.y});
               })
             | future::Map([](Widget w) {
                 return result::Ok<Widget, Gadget>(std::move(w));
               })
             | future::AndThen([](Widget w) {
                 return future::Err<Widget, Gadget>(Gadget{w.x});
               })
             | future::OrElse([](Gadget g) {
                 return future::Ok<Widget, Gadget>(Widget{g.y});
               })
             | future::MapOk([](Widget w) {
                 return Widget{w.x + 1};
               });

    auto r = future::Get(std::move(f));
    ASSERT_TRUE(r);
    auto w = *r;
    ASSERT_EQ(w.x, 2);

    mt.Stop();
  }

  SIMPLE_TEST(MoveOnly3) {
    auto f = future::Ok<MoveOnly, int>({})
             | future::MapOk([](MoveOnly) {
                 return MoveOnly{};
               })
             | future::AndThen([](MoveOnly) {
                 return future::Err<MoveOnly, int>({});
               })
             | future::OrElse([](int) {
                 return future::Ok<MoveOnly, int>({});
               });

    future::Get(std::move(f));
  }

  SIMPLE_TEST(MoveOnly4) {
    auto f = future::Value<MoveOnly>({})
             | future::Map([m = MoveOnly{}](MoveOnly) mutable {
                 return std::move(m);
               })
             | future::FlatMap([m = MoveOnly{}](MoveOnly) mutable {
                 return future::Value(std::move(m));
               })
             | future::Map([](MoveOnly m) {
                 return result::Ok<MoveOnly, std::error_code>(std::move(m));
               })
             | future::MapOk([m = MoveOnly{}](MoveOnly) mutable {
                 return std::move(m);
               })
             | future::AndThen([m = MoveOnly{}](MoveOnly) mutable {
                 [[maybe_unused]] auto u = std::move(m);
                 return future::Err<MoveOnly, std::error_code>(TimeoutError());
               })
             | future::OrElse([m = MoveOnly{}](std::error_code) mutable {
                 return future::Ok<MoveOnly, std::error_code>(std::move(m));
               });

    future::Get(std::move(f));
  }

  SIMPLE_TEST(Via2) {
    runtime::Sandbox loop1;
    runtime::Sandbox loop2;

    size_t steps = 0;

    future::Just()
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([](Unit) {
            return unit;
          })
        | future::Via(loop2)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            return unit;
          })
        | future::FlatMap([&](Unit) {
            ++steps;
            return future::Value(1);
          })
        | future::Map([&](int v) {
            ASSERT_EQ(v, 1);
            ++steps;
            return unit;
          })
        | future::Detach();

    ASSERT_LE(loop1.RunTasks(), 2);
    ASSERT_EQ(steps, 1);
    loop2.RunTasks();
    ASSERT_EQ(steps, 3);
    loop1.RunTasks();
    ASSERT_EQ(steps, 6);
  }

  SIMPLE_TEST(Move) {
    runtime::MultiThread mt{4};
    mt.Start();

    auto f = future::Just() | future::Via(mt) | future::Map([](Unit) {
      return 7;
    });

    auto ff = std::move(f);

    auto fff = std::move(ff);

    auto g = std::move(fff) | future::Map([&mt](int) {
      ASSERT_TRUE(mt.Here());
      return unit;
    });

    future::Get(std::move(g));

    mt.Stop();
  }

  SIMPLE_TEST(First1) {
    auto [f1, p1] = future::Contract<int>();
    auto [f2, p2] = future::Contract<int>();

    auto first = future::First(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(first)
        | future::Map([&ok](int v) {
            ASSERT_EQ(v, 1);
            ok = true;
            return unit;
          })
        | future::Detach();

    std::move(p1).Set(1);
    // std::move(p2).Set(2);

    ASSERT_TRUE(ok);

    std::move(p2).Set(2);
  }

  SIMPLE_TEST(First2) {
    auto [f1, p1] = future::Contract<int>();
    auto [f2, p2] = future::Contract<int>();

    auto first = future::First(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(first)
        | future::Map([&ok](int v) {
            ASSERT_EQ(v, 2);
            ok = true;
            return unit;
          })
        | future::Detach();

    std::move(p2).Set(2);

    ASSERT_TRUE(ok);

    std::move(p1).Set(1);
  }

  SIMPLE_TEST(FirstOk1) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto first = future::FirstOk(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(first) 
      | future::MapOk([&ok](int v) {
          ASSERT_EQ(v, 2);
          ok = true;
          return unit;
        }) 
      | future::Detach();

    std::move(p2).Set(result::Ok<int, std::error_code>(2));

    ASSERT_TRUE(ok);

    std::move(p1).Set(result::Ok<int, std::error_code>(1));
  }

  SIMPLE_TEST(FirstOk2) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto first = future::FirstOk(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(first) 
      | future::MapOk([&ok](int v) {
          ASSERT_EQ(v, 29);
          ok = true;
          return unit;
        }) 
      | future::Detach();

    std::move(p1).Set(result::Err<int>(TimeoutError()));

    ASSERT_FALSE(ok);

    std::move(p2).Set(result::Ok<int, std::error_code>(29));

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(FirstOk3) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto first = future::FirstOk(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(first) 
        | future::MapOk([&ok](int v) {
            ASSERT_EQ(v, 31);
            ok = true;
            return unit;
          }) 
        | future::Detach();

    std::move(p2).Set(result::Err<int>(IoError()));

    ASSERT_FALSE(ok);

    std::move(p1).Set(result::Ok<int, std::error_code>(31));

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(FirstFailure) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto first = future::FirstOk(std::move(f1), std::move(f2));

    bool fail = false;

    std::move(first) 
        | future::OrElse([&](std::error_code e) {
            ASSERT_EQ(e, TimeoutError());
            fail = true;
            return future::Err<int>(e);
          }) 
        | future::Detach();

    std::move(p2).Set(result::Err<int>(IoError()));

    ASSERT_FALSE(fail);

    std::move(p1).Set(result::Err<int>(TimeoutError()));

    ASSERT_TRUE(fail);
  }

  SIMPLE_TEST(Both) {
    auto [f1, p1] = future::Contract<int>();
    auto [f2, p2] = future::Contract<int>();

    auto both = future::Both(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(both)
        | future::Map([&ok](auto tuple) {
            auto [x, y] = tuple;
            ASSERT_EQ(x, 1);
            ASSERT_EQ(y, 2);
            ok = true;
            return unit;
          })
        | future::Detach();

    std::move(p2).Set(2);

    ASSERT_FALSE(ok);

    std::move(p1).Set(1);

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(BothOk) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto both = future::BothOk(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(both) 
        | future::MapOk([&ok](auto tuple) {
            auto [x, y] = tuple;
            ASSERT_EQ(x, 2);
            ASSERT_EQ(y, 1);
            ok = true;
            return unit;
          }) 
        | future::Detach();

    std::move(p2).Set(result::Ok<int, std::error_code>(1));

    ASSERT_FALSE(ok);

    std::move(p1).Set(result::Ok<int, std::error_code>(2));

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(BothFailure1) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto both = future::BothOk(std::move(f1), std::move(f2));

    bool fail = false;

    std::move(both) 
        | future::OrElse([&fail](std::error_code e) {
            ASSERT_EQ(e, TimeoutError());
            fail = true;
            return future::Err<std::tuple<int, int>>(e);
          }) 
        | future::Detach();

    std::move(p1).Set(result::Err<int>(TimeoutError()));

    ASSERT_TRUE(fail);

    std::move(p2).Set(result::Ok<int, std::error_code>(7));
  }

  SIMPLE_TEST(BothFailure2) {
    auto [f1, p1] = future::Contract<Result<int, std::error_code>>();
    auto [f2, p2] = future::Contract<Result<int, std::error_code>>();

    auto both = future::BothOk(std::move(f1), std::move(f2));

    bool fail = false;

    std::move(both) 
        | future::OrElse([&fail](std::error_code e) {
            ASSERT_EQ(e, IoError());
            fail = true;
            return future::Err<std::tuple<int, int>>(e);
          }) 
        | future::Detach();

    std::move(p2).Set(result::Err<int>(IoError()));

    ASSERT_TRUE(fail);

    std::move(p1).Set(result::Ok<int, std::error_code>(4));
  }

  SIMPLE_TEST(BothTypes) {
    auto [f1, p1] = future::Contract<std::string>();
    auto [f2, p2] = future::Contract<std::tuple<int, int>>();

    auto both = future::Both(std::move(f1), std::move(f2));

    bool ok = false;

    std::move(both) 
        | future::Map([&ok](auto tuple) {
            auto [x, y] = tuple;

            ASSERT_EQ(x, "3");

            std::tuple<int, int> t = {1, 2};

            ASSERT_TRUE(y == t);

            ok = true;
            return unit;
          }) 
        | future::Detach();

    std::move(p2).Set({1, 2});
    std::move(p1).Set("3");

    ASSERT_TRUE(ok);
  }

  SIMPLE_TEST(DoNotWait1) {
    runtime::MultiThread mt{4};
    mt.Start();

    std::atomic_bool submit = false;

    auto f = future::Spawn(mt,
                           [&] {
                             std::this_thread::sleep_for(2s);
                             submit = true;
                             return 11;
                           })
             | future::Map([](int v) {
                 return v + 1;
               });

    ASSERT_FALSE(submit);  // Unreliable

    auto v = future::Get(std::move(f));

    ASSERT_TRUE(submit);
    ASSERT_EQ(v, 12);

    mt.Stop();
  }

  SIMPLE_TEST(DoNotWait2) {
    runtime::MultiThread mt{4};
    mt.Start();

    std::atomic_bool submit = false;

    auto f = future::Spawn(mt,
                           [&] {
                             std::this_thread::sleep_for(2s);
                             submit = true;
                             return 31;
                           })
             | future::FlatMap([&](int v) {
                 return future::Spawn(mt, [v] {
                   return v + 1;
                 });
               });

    ASSERT_FALSE(submit);

    auto v = future::Get(std::move(f));

    ASSERT_TRUE(submit);
    ASSERT_EQ(v, 32);

    mt.Stop();
  }

  SIMPLE_TEST(Inline1) {
    runtime::Sandbox loop;

    bool ok = false;

    future::Just()
        | future::Via(loop)
        | future::Map([&](Unit) {
            ok = true;
            return unit;
          })
        | future::Detach();

    size_t tasks = loop.RunTasks();
    ASSERT_TRUE(ok);
    ASSERT_EQ(tasks, 1);
  }

  SIMPLE_TEST(Inline2) {
    runtime::Sandbox loop;

    bool flat_map = false;
    bool map1 = false;
    bool map2 = false;

    future::Just()
        | future::Via(loop)
        | future::FlatMap([&](Unit) {
            flat_map = true;
            return future::Just();
          })
        | future::Map([&](Unit u) {
            map1 = true;
            return u;
          })
        | future::Map([&](Unit u) {
            map2 = true;
            return u;
          })
        | future::Detach();

    ASSERT_TRUE(loop.RunNextTask());
    ASSERT_TRUE(flat_map);

    loop.RunNextTask();
    ASSERT_TRUE(map1);
    loop.RunTasks();
    ASSERT_TRUE(map2);
  }

  SIMPLE_TEST(Inline3) {
    runtime::Sandbox loop;

    future::Spawn(loop, [&] {
      return future::Spawn(loop, [] {
        return 19;
      });
    }) | future::Flatten() | future::Detach();

    size_t tasks = loop.RunTasks();
    ASSERT_EQ(tasks, 2);
  }
}

RUN_ALL_TESTS()
