#include <exe/runtime/multi_thread.hpp>

#include <exe/future/make/contract.hpp>
#include <exe/future/make/spawn.hpp>
// #include <exe/future/make/run.hpp>
#include <exe/future/make/value.hpp>
#include <exe/future/make/return.hpp>
#include <exe/future/make/just.hpp>

#include <exe/future/combine/seq/map.hpp>
#include <exe/future/combine/seq/flatten.hpp>
#include <exe/future/combine/seq/flat_map.hpp>
#include <exe/future/combine/seq/via.hpp>
#include <exe/future/combine/seq/after.hpp>

#include <exe/future/terminate/get.hpp>
#include <exe/future/terminate/detach.hpp>

#include <exe/result/make/ok.hpp>
#include <exe/result/make/err.hpp>

#include <exe/result/combine/map.hpp>
#include <exe/result/combine/and_then.hpp>
#include <exe/result/combine/or_else.hpp>

#include <exe/result/syntax/pipe.hpp>

#include <exe/future/make/result/ok.hpp>
#include <exe/future/make/result/err.hpp>

#include <exe/future/combine/seq/result/map_ok.hpp>
#include <exe/future/combine/seq/result/and_then.hpp>
#include <exe/future/combine/seq/result/or_else.hpp>

#include <exe/future/combine/concur/first.hpp>

#include <exe/future/syntax/or.hpp>

#include <fmt/core.h>

#include <thread>


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

using namespace std::chrono_literals;

using namespace exe;  // NOLINT

struct ExampleError {
  //
};
#include <iostream>
int main() {
  // runtime::Sandbox loop1;
  // runtime::Sandbox loop2;
  // auto f1 = future::Just()
  //       | future::Via(loop1)
  //       | future::Map([&](Unit) {
  //           ++steps;
  //           return unit;
  //         })
  //       | future::Map([](Unit) {
  //           return unit;
  //         });
  // assert(f1.GetRuntime() == loop1);

  runtime::Sandbox loop1;
    runtime::Sandbox loop2;

    size_t steps = 0;

    future::Just()
        | future::Via(loop1)
        | future::Map([&](Unit) {
            ++steps;
            std::cout << "1" << std::endl;
            return unit;
          })
        | future::Map([](Unit) {
          std::cout << "2" << std::endl;
            return unit;
          })
        | future::Via(loop2)
        | future::Map([&](Unit) {
          std::cout << "3" << std::endl;
            ++steps;
            return unit;
          })
        | future::Map([&](Unit) {
          std::cout << "4" << std::endl;
            ++steps;
            return unit;
          })
        | future::Via(loop1)
        | future::Map([&](Unit) {
          std::cout << "5" << std::endl;
            ++steps;
            return unit;
          })
        | future::Map([&](Unit) {
          std::cout << "6" << std::endl;
            ++steps;
            return 1;
          })
        | future::Map([&](int v) {
            std::cout << "7" << std::endl;
            ASSERT_EQ(v, 1);
            ++steps;
            return unit;
          })
        | future::Detach();
      std::cout << "before rt1" << std::endl;
    ASSERT_LE(loop1.RunTasks(), 2);
      std::cout << "before rt2" << std::endl;
    ASSERT_EQ(steps, 1);
    loop2.RunTasks();
     std::cout << "before rt3" << std::endl;
    ASSERT_EQ(steps, 3);
    loop1.RunTasks();
    ASSERT_EQ(steps, 6);
  // runtime::MultiThread rt{4};
  // rt.WithTimers().Start();

  // {
  //   // Contract
  //   auto [f, p] = future::Contract<int>();

  //   std::move(p).Set(1);
  //   auto v = future::Get(std::move(f));

  //   fmt::println("Contract -> {}", v);
  // }

  // {
  //   // Spawn (Run)

  //   auto f = future::Spawn(rt, [] {
  //     fmt::println("Running on thread pool");
  //     return 7;
  //   });

  //   int v = future::Get(std::move(f));

  //   fmt::println("Spawn -> {}", v);
  // }

  // {
  //   // Value

  //   auto f = future::Value(42);

  //   auto v = future::Get(std::move(f));

  //   fmt::println("Value -> {}", v);
  // }

  // /*
  //  * Pipeline operator
  //  *
  //  * Output(Multiply(Add(Input(), 2), 5))
  //  *
  //  * x |> f(y) ~ f(x, y)
  //  *
  //  * Input() |> Add(2) |> Multiply(5) |> Output()
  //  *
  //  */


  // {
  //   // Spawn + Map

  //   // Map: Future<T> -> (T -> U) -> Future<U>

  //   auto f = future::Spawn(rt, [] {
  //              fmt::println("Running on thread pool");
  //              return 1;
  //            })
  //            | future::Map([](int v) {
  //                return v + 1;
  //              });

  //   auto v = future::Get(std::move(f));

  //   fmt::println("Spawn.Map -> {}", v);
  // }

  // {
  //   // Just [unit]

  //   auto f = future::Just();

  //   [[maybe_unused]] Unit u = future::Get(std::move(f));
  // }

  // {
  //   // Spawn = Just | Via | Map

  //   auto f = future::Just()
  //            | future::Via(rt)
  //            | future::Map([](Unit) {
  //                fmt::println("Running on thread pool");
  //                return unit;
  //              });

  //   future::Get(std::move(f));
  // }

  // {
  //   // Flatten

  //   // Future<Future<T>> -> Future<T>

  //   // LTL: ♢♢phi = ♢phi

  //   auto f = future::Just()
  //            | future::Via(rt)
  //            | future::Map([](Unit) {
  //                return future::Value(7);
  //              })
  //            // | future::Map([](int v) { return v + 1; })
  //            | future::Flatten()
  //            | future::Map([](int v) { return v + 1; });

  //   int v = future::Get(std::move(f));

  //   fmt::println("Flatten -> {}", v);
  // }

  // {
  //   // FlatMap: Future<T> -> (T -> Future<U>) -> Future<U>

  //   // A ; B
  //   // FlatMap | FlatMap

  //   auto f = future::Spawn(rt, [] { return 1; })
  //            | future::FlatMap([](int v) {
  //                return future::Value(v + 1);
  //              })
  //            | future::Map([](int v) {
  //                return v + 1;
  //              });

  //   auto v = future::Get(std::move(f));

  //   fmt::println("FlatMap -> {}", v);  // 3
  // }

  // {
  //   /*
  //  * Monadic Result<T, E>
  //  *
  //  * Map: Result<T, E> -> (T -> U) -> Result<U, E>
  //  * AndThen: Result<T, E> -> (T -> Result<U, E>) -> Result<U, E>
  //  * OrElse: Result<T, E> -> (E -> Result<T, E>) -> Result<T, E>
  //  *
  //    */

  //   {
  //     // Happy path

  //     result::Ok<int, ExampleError>(1)
  //         | result::Map([](int v) {
  //             return v * 2;
  //           })
  //         | result::AndThen([](int v) {
  //             return result::Ok<int, ExampleError>(v + 1);
  //           })
  //         | result::OrElse([](ExampleError /*error*/) {  // Skipped
  //             return result::Ok<int, ExampleError>(42);  // Fallback
  //           })
  //         | result::Map([](int v) {
  //             fmt::println("Ok.AndThen.OrElse -> {}", v);
  //           });
  //   }

  //   {
  //     // Fallback

  //     result::Err<int, ExampleError>({})
  //         | result::Map([](int v) {  // Skipped
  //             return v * 2;
  //           })
  //         | result::AndThen([](int v) {  // Skipped
  //             return result::Ok<int, ExampleError>(v + 1);
  //           })
  //         | result::OrElse([](ExampleError /*error*/) {
  //             return result::Ok<int, ExampleError>(42);  // Fallback
  //           })
  //         | result::Map([](int v) {
  //             fmt::println("Ok.AndThen.OrElse -> {}", v);
  //           });
  //   }
  // }

  // {
  //   struct ExampleError {
  //     int code;
  //   };

  //   auto f = future::Ok<int, ExampleError>(1)
  //            | future::Via(rt)
  //            | future::AndThen([](int) mutable {
  //                return future::Err<int, ExampleError>({});
  //              })
  //            | future::AndThen([](int) {
  //                //std::abort();  // Unreachable
  //                return future::Ok<int, ExampleError>(1);
  //              })
  //            | future::OrElse([](ExampleError) {
  //                return future::Ok<int, ExampleError>(42);  // Fallback
  //              })
  //            | future::MapOk([](int v) {
  //                return v + 1;
  //              });

  //   auto r = future::Get(std::move(f));

  //   std::move(r) | result::Map([](int v) {
  //     fmt::println("AndThen.AndThen.OrElse.MapOk -> {}", v);
  //   });
  // }

  // {
  //   // Synonym: Race

  //   auto slow = future::Just()
  //               | future::Via(rt)
  //               | future::After(1s)
  //               | future::Map([](Unit) {
  //                   return 1;
  //                 });

  //   auto fast = future::Just()
  //               | future::Via(rt)
  //               | future::Map([](Unit) {
  //                   return 2;
  //                 });

  //   // auto first = future::First(std::move(slow), std::move(fast));
  //   auto first = std::move(slow) or std::move(fast);

  //   auto v = future::Get(std::move(first));

  //   fmt::println("First -> {}", v);

  //   std::this_thread::sleep_for(2s);  // Better?
  // }

  // rt.Stop();

  return 0;
}
