# (Functional) Future

## Пререквизиты

- [runtime/timer](/tasks/runtime/timer)
- [future/contract](/tasks/future/contract)
- [рекомендуется] [fiber/mutex](/tasks/fiber/mutex)

---

До этого момента мы писали (и думали о) concurrency в императивном стиле, представляя конкурентные активности как цепочки шагов-мутаций разделяемого состояния и называя эти цепочки _файберами_. 

В этой задаче предлагается [функциональная](https://www.haskell.org/) модель (и язык) для concurrency, в которой конкурентные активности
представлены как графы трансформаций иммутабельных значений и именуются _фьючами_ (_futures_).

## `Future`

В основе модели лежит понятие _future_ (далее – просто _фьюча_) и шаблон `Future<T>`.

`Future<T>` представляет будущее (в общем случае – еще не готовое) **значение**, вычислямое асинхронной операцией. 

За фьючей может стоять
- [вычисление в пуле потоков](source/exe/future/make/spawn.hpp),
- [RPC](https://capnproto.org/cxxrpc.html),  
- [IO](https://tokio.rs/tokio/tutorial/io),
- [ожидание на семафоре](https://github.com/twitter/util/blob/143adbdf5bc6dc55eddb9248c597eeb11a799094/util-core/src/main/scala/com/twitter/concurrent/AsyncSemaphore.scala#L141),
- [таймаут](https://gitlab.com/Lipovsky/await/-/blob/3ff428941d6ccf414e0e4c9fd3831f883c6379f5/demo/main.cpp#L44) и т.д.

## `Result`

Фьюча может представлять вызов метода на удаленном сервере или чтение с диска,
и нужно учитывать, что подобная операция может завершиться ошибкой.

Результат для таких (_fallible_, допускающих сбой) операций мы будем представлять в виде `Result<T, E>`, который содержит либо значение типа `T`, либо ошибку типа `E`.

Положим [`TryFuture<T, E>`](source/exe/future/type/try.hpp) = `Future<Result<T, E>>` – _сбойная_ фьюча.

## Комбинаторы

Фьючи, т.е. стоящие за ними асинхронные операции, мы будем комбинировать в функциональном стиле:

```cpp
future::TryFuture<Response, Error> Hedge(Request request) {
  // primary – сбойная фьюча, представляющая удаленный вызов к некоторому сервису
  auto primary = Rpc(request);

  // "Запасной" запрос к другой реплике сервиса, выполняемый по истечению таймаута
  auto backup = future::Just() | future::After(99ms) | future::FlatMap([request](Unit) {
    return Rpc(request);
  });
  
  // Возвращаем первый успешный ответ (а что делать с опоздавшим запросом?)
  return future::FirstOk(std::move(primary), std::move(backup));
}
``` 

На этом небольшом примере с _хэджированием_ RPC (см. статью [Tail at Scale](https://www.barroso.org/publications/TheTailAtScale.pdf)) хорошо видны преимущества функционального подхода в приложении к concurrency:

Разработчик описывает **что** и **когда** он хочет сделать, а за реализацию этого плана (и за всю сопряженную с ним синхронизацию) отвечают комбинаторы.

**Меняется ментальная модель** concurrency: разработчик думает не про чередование обращений к разделяемому состоянию, т.е. не про control flow, а про **трансформацию иммутабельных значений**, которые "текут" от продьюсеров к консьюмерам, т.е. про **data flow**.

### References

- [Your Server as a Function](https://monkey.org/~marius/funsrv.pdf) &
- [Futures aren't Ersatz Threads](https://monkey.org/~marius/futures-arent-ersatz-threads.html) by Marius Eriksen

## Модель

Фьючи – значения, которые представляют цепочки / графы задач или (в общем случае) – произвольные конкурентные активности.

### Операции

- _конструкторы_ (`Contract`, `Spawn`, `Value`, ...) или _сервисы_ строят фьючи / открывают цепочки,
- _комбинаторы_ (`Map`, `FlatMap`, `AndThen`, `OrElse`, ...) получают фьючи на вход, поглощают их и строят новые фьючи / продолжают цепочки,
- _терминаторы_ (`Get`, `Detach`) поглощают фьючи / завершают цепочки.

_Конструкторы_ моделируют продьюсеров, _терминаторы_ – консьюмеров.

Вместе они описывают графы операций / вычислений / задач.

### Линейность

Фьючи – _линейные_ (_linear_): каждая фьюча должна быть поглощена (комбинатором или терминатором) **ровно один раз** (exactly-once).

#### C++

Линейность нельзя выразить в системе типов С++.

В качестве приближения потребуем: фьюча не может быть скопирована (_non-copyable_), только перемещена (_move-constructible_, _non-move-assignable_).

Будем считать, что непотребленная фьюча – это [implementation-defined behavior](https://eel.is/c++draft/defns.impl.defined).

## Пример

```cpp
void FuturesExample() {
  using namespace exe;
  
  runtime::MultiThread rt{4};
  rt.Start();
  
  auto pipeline = future::Just()  // <- конструктор, начинает цепочку
      | future::Via(rt) 
      | future::Map([](Unit) {  // <- комбинатор
          fmt::println("Running on thread pool");
          return 42;
        })
      | future::After(1s)
      | future::FlatMap([](int) {
          return future::Err<int, std::error_code>(TimeoutError());
      })
      | future::OrElse([](std::error_code) {
          return future::Ok<int, std::error_code>(7);  // Fallback
        })
      | future::MapOk([](int v) {
          return v + 1;
        })
      | future::MapOk([](int v) {
          fmt::println("{}", v);
          return unit;
        });
      
  future::Get(std::move(pipeline));  // <- терминатор
  // Напечатано 8
  
  rt.Stop();
}
```

Больше примеров – в [play/main.cpp](play/main.cpp)


## Структура `exe/future`

```sh
exe
├── type # Типы (Future, TryFuture)
├── make # Конструкторы (Spawn, Value, Just, ...)
├── combine # Комбинаторы
│   ├── seq # Последовательная композиция (Map, FlatMap, ...)
│   ├── concur # Конкурентная композиция (First, All)
├── terminate # Терминаторы (Get, Detach)
└── syntax # Перегрузки операторов: |, or, *
```

## Конструкторы

_Конструкторы_ – функции, конструирующие новые фьючи.

Каталог базовых конструкторов: [`future/make`](source/exe/future/make)

Заданный набор конструкторов подразумевает расширение: IO, RPC, etc.

---

### 👉 `Contract`

Заголовок: [`make/contract.hpp`](source/exe/future/make/contract.hpp)

Асинхронный контракт между продьюсером и консьюмером:

```cpp
auto [f, p] = future::Contract<int>();

// Producer
std::thread producer([p = std::move(p)] mutable {
  std::move(p).Set(7);
});

// Consumer
auto v = future::Get(std::move(f));  // 7

producer.join();
```

Обязанности сторон:

- продьюсер **обязан** выполнить фьючу, вызвать `Set` на `Promise<T>`,
- консьюмер **обязан** потребить фьючу с помощью комбинатора или терминатора.

---

### 👉 `Spawn`

Заголовок: [`make/spawn.hpp`](source/exe/future/make/spawn.hpp)

Фьюча, представляющая вычисление в рантайме.

```cpp
auto f = future::Spawn(rt, [] {
  return 7;
});
```

---

### 👉 `Run`

Заголовок: [`make/run.hpp`](source/exe/future/make/run.hpp)

Фьюча, представляющая вычисление в рантайме.

Синоним для `Spawn`.

```cpp
auto f = future::Run(rt, [] {
  return 7;
});
```

---

### 👉 `Ready`

Заголовок: [`make/ready.hpp`](source/exe/future/make/ready.hpp)

Фьюча, представляющая готовое значение.

Темпоральная константа.

```cpp
auto f = future::Ready(result::Ok<int, std::error_code>(2));
```

---

### 👉 `Value`

Заголовок: [`make/value.hpp`](source/exe/future/make/value.hpp)

Аналогично `Ready`, но только для простых значений: `Result<T, E>` в качестве значения не допускается.

Существует для повышения читаемости пользовательского кода.

```cpp
auto f = future::Value(7);
```

---

### 👉 `Return`

Заголовок: [`make/return.hpp`](source/exe/future/make/return.hpp)

Синоним для `Ready`, отсылающий читателя к [монадам](https://wiki.haskell.org/Typeclassopedia).

```cpp
auto f = future::Return(7);
```

---

### 👉 `Just`

Заголовок: [`make/just.hpp`](source/exe/future/make/just.hpp)

Фьюча, представляющая готовое значение типа [`Unit`](source/exe/unit.hpp).

```cpp
auto f = future::Just()
    | future::Via(rt) 
    | future::Map([](Unit) {
        return 42;
      });
```

#### Unit

Асинхронные операции, не возвращающие значения, моделируются с помощью `Future<Unit>`: 

https://en.wikipedia.org/wiki/Unit_type

Фьючи – это будущие значения, но значений типа `void` не бывает, значит не должно быть и будущих `void`.

## Комбинаторы

_Комбинаторы_ – функции, которые получают фьючи на вход (потребляют их) и строят новые фьючи.

### Последовательная композиция

Каталог: [`future/combine/seq`](source/exe/future/combine/seq)

#### Pipeline operator

Последовательная композиция фьюч на уровне синтаксиса выражается через оператор `|` (_pipeline operator_). 

Оператор не имеет прямого отношения к фьючам, он решает более общую задачу: упростить описание цепочек вызовов функций, где последующий вызов получает на вход выход предыдущего. 

Выражение `f(a) | h(x, y)` переписывается оператором `|` в `h(f(a), x, y)`

Поддержка `|` для фьюч: [future/syntax/pipe.hpp](source/exe/future/syntax/pipe.hpp)

##### References

С помощью перегрузки оператора `|` мы эмулируем "настоящий" (и не существующий в языке С++) оператор `|>`:

- [OCaml] [Pipelining](https://cs3110.github.io/textbook/chapters/hop/pipelining.html)
- [JavaScript] [Pipe Operator (|>) for JavaScript](https://github.com/tc39/proposal-pipeline-operator/)
- [C++] [Exploring the Design Space for a Pipeline Operator](https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2022/p2672r0.html)

---

#### 👉 `Map` 

Заголовок: [`combine/seq/map.hpp`](source/exe/future/combine/seq/map.hpp)

Сигнатура: `Future<T>` → (`T` → `U`) → `Future<U>` ([нотация](https://www.haskell.org/tutorial/functions.html) заимствована из Haskell)

Комбинатор `Map` применяет (в будущем) функцию пользователя (_маппер_) к значению входной фьючи:

```cpp
auto f = future::Value(1) | future::Map([](int v) {
  return v + 1;
});
```

Как и любой другой комбинатор, `Map` применяется к фьюче **без ожидания** будущего результата, он лишь планирует вызов продолжения через подписку на результат входной фьючи и строит новую (выходную) фьючу.

---

#### 👉 `Via`

Заголовок: [`combine/seq/via.hpp`](source/exe/future/combine/seq/via.hpp)

Устанавливает рантайм для последующих шагов асинхронного вычисления

```cpp
auto f = future::Just() | future::Via(rt) | future::Map([] { ... });
```

Установленный через `Via` рантайм **наследуется** по цепочке (пока не встретит параллельный комбинатор или новый `Via`).

##### `Inline`

Рантайм по умолчанию – [`Inline`](source/exe/runtime/inline.hpp).

##### State

Можно заметить, что `Via` – это мутация состояния, которая "просочилась" в _чистую_ (_pure_) функциональную модель.

---

#### 👉 `Flatten`

Заголовок: [`combine/seq/flatten.hpp`](source/exe/future/combine/seq/flatten.hpp)

Сигнатура: `Future<Future<T>>` → `Future<T>`

LTL: ♢♢ = ♢ ([A Science of Concurrent Programs](https://lamport.azurewebsites.net/tla/science.pdf))

"Уплощает" вложенную асинхронность:

```cpp
// f :: Future<int>
auto f = future::Spawn(rt, [] { return future::Value(7); }) 
          | future::Flatten();
```

---

#### 👉 `FlatMap`

Заголовок: [`combine/seq/flat_map.hpp`](source/exe/future/combine/seq/flat_map.hpp)

Сигнатура: `Future<T>` → (`T` → `Future<U>`) → `Future<U>`

Добавляет к цепочке асинхронный шаг:

```cpp
// f :: Future<int>
auto f = future::Value(1)
    | future::Via(rt) 
    | future::FlatMap([](int v) {
        return future::Value(v + 1);
      });   
```

##### Монады

Фьючи с конструктором `Return` (`return`) и комбинатором `FlatMap` (`>>=`) образуют [монаду](https://wiki.haskell.org/All_About_Monads#The_Monad_class).

---

#### 👉 `After`

Заголовок: [`combine/seq/after.hpp`](source/exe/future/combine/seq/after.hpp)

Сигнатура: `Future<T>` → `std::chrono::microseconds` → `Future<T>`

Задерживает следующий шаг на заданное время.

Асинхронный аналог `SleepFor`.

```cpp
auto f = future::Just()
    | future::Via(rt)
    | future::After(1s)
    | future::Map([](Unit) {
        return 7;
      });
```

---

#### `TryFuture<T, E>`

Комбинаторы для `TryFuture<T, E>`, представляющей сбойные операции.

##### 👉 `AndThen` / `OrElse`

Заголовки:
- [`combine/seq/result/and_then.hpp`](source/exe/future/combine/seq/result/and_then.hpp)
- [`combine/seq/result/or_else.hpp`](source/exe/future/combine/seq/result/or_else.hpp)

Сигнатура:
- `AndThen`: `TryFuture<T, E>` → (`T` → `TryFuture<U, E>`) → `TryFuture<U, E>`
- `OrElse`: `TryFuture<T, E>` → (`E` → `TryFuture<T, E>`) → `TryFuture<T, E>`

Комбинаторы `AndThen` / `OrElse` разделяют успешный путь / обработку ошибки в цепочке сбойных асинхронных шагов:
- `AndThen` вызывается только на значениях,
- `OrElse` – только на ошибках.

```cpp
    auto f = future::Ok<int, ExampleError>(1)
             | future::AndThen([](int) mutable {
                 return future::Err<int, ExampleError>({});
               })
             | future::AndThen([](int) {
                 std::unreachable();
                 return future::Ok<int, ExampleError>(1);
               })
             | future::OrElse([](ExampleError) {
                 return future::Ok<int, ExampleError>(42);  // Fallback
               })
             | future::MapOk([](int v) {
                 return v + 1;
               });
```

Можно заметить, что `AndThen` / `OrElse` – это асинхронный аналог для блока `try` / `catch`.

##### 👉 `MapOk`

Заголовок: [`combine/seq/result/map_ok.hpp`](source/exe/future/combine/seq/result/map_ok.hpp)

Сигнатура: `TryFuture<T, E>` → (`T` → `U`) → `TryFuture<U, E>`

Вариация `Map` для успешного пути в сбойном вычислении:

```cpp
auto f = future::Ready(result::Ok<int, int>(1))
          | future::MapOk([](int v) {
              return v + 2;
            });
```

### Concurrent композиция

Каталог: [`combine/concur`](source/exe/future/combine/concur)

---

#### 👉 `First`

Заголовок: [`combine/concur/first.hpp`](source/exe/future/combine/concur/first.hpp)

Фьюча, представляющая первое значение фьюч, поданных на вход.

```cpp
auto timeout = future:::Just() 
    | future::Via(rt) 
    | future::After(1s) 
    | future::Map([](Unit) {
        return result::Err<int, std::error_code>(TimeoutError());
      });

auto compute = future::Spawn(rt, [] {
  return result::Ok<int, std::error_code>(7);  // ~ Вычисление
});

// Вычисление с таймаутом
auto compute_with_timeout = future::First(std::move(compute), std::move(timeout));

// Первый готовый Result
auto result = future::Get(std::move(compute_with_timeout));
```

Тип значения входных фьюч не интерпретируется.

#### 👉 `FirstOk`

Заголовок: [`combine/concur/result/first.hpp`](source/exe/future/combine/concur/result/first.hpp)

Вариация `First` для `TryFuture`.

Фьюча, представляющая первый успех / последнюю ошибку двух сбойных фьюч, поданных на вход.

См. пример с хеджированием RPC.

---

#### 👉 `Both` / `All`

Заголовок: [`combine/concur/all.hpp`](source/exe/future/combine/concur/all.hpp)

Фьюча, представляющая пару (кортеж) значений фьюч, поданных на вход:

```cpp
auto f = future::Spawn(rt, [] { return 1; });
auto g = future::Spawn(rt, [] { return 2; });

// Без ожидания
auto both = future::Both(std::move(f), std::move(g));

// Синхронно ожидаем двух значений
auto [x, y] = future::Get(std::move(both));
```

Тип значения входных фьюч не интерпретируется.

##### 👉 `BothOk` / `AllOk` 

Вариация `Both` для `TryFuture`.

Фьюча, представляющая оба (все) значения / первую ошибку сбойных фьюч, поданных на вход.

##### Short-circuit

Синхронная распаковка фьючи `BothOk` **не эквивалентна** последовательной синхронной распаковке
двух фьюч `f` и `g`: если вторая фьюча завершилась ошибкой, то ожидание первой будет прервано.

---

#### Контекст

Параллельные комбинаторы **сбрасывают** рантайм до `Inline`, так что пользователь должен явно установить его после объединения цепочек.

## Терминаторы

_Терминаторы_ завершают асинхронные цепочки / графы.

Каталог: [`future/terminate`](source/exe/future/terminate)

---

### 👉 `Get`

Заголовок: [`terminate/get.hpp`](source/exe/future/terminate/get.hpp)

Терминатор `Get` блокирует текущий поток до выполнения фьючи:

```cpp
// Планируем задачу в рантайм
auto f = future::Spawn(rt, [] {
  return 7;
});

// Блокируем поток до готовности значения
auto r = future::Get(std::move(f));
```

Иначе говоря, `Get` синхронно "распаковывает" `Future` в её `ValueType`.

#### Chaining

Терминатор `Get` можно использовать с оператором `|`:

```cpp
auto r = future::Spawn(rt, [] { return 7; })
          | future::Get();
```

---

### 👉 `Detach`

Заголовок: [`terminate/detach.hpp`](source/exe/future/terminate/detach.hpp)

`Future` аннотирована как [`[[nodiscard]]`](https://en.cppreference.com/w/cpp/language/attributes/nodiscard) и должна быть явно поглощена консьюмером. 

Терминатор `Detach` поглощает фьючу и игнорирует ее результат.

```cpp
// Завершение вычисления нас не интересует
future::Spawn(rt, [] { /* ... */ }) 
    | future::Detach();  // Considered harmful!
```

## Операторы

В [`future/syntax`](source/exe/future/syntax) собраны перегрузки операторов для фьюч:

- `f | c` означает `c.Pipe(f)`
- `f or g` означает `First(f, g)`
- `f * g` означает `Both(f, g)`

## Лямбды

Эргономика фьюч сильно зависит от эргонимики анонимных функций.

Сравните:
- `[](int v) { return v + 1; }` (C++)
- `|v| v + 1` ([Rust](https://doc.rust-lang.org/rust-by-example/fn/closures.html))
- `_ + 1` ([Scala](https://docs.scala-lang.org/scala3/book/fun-anonymous-functions.html))

## Значения

Как вы можете видеть, пользователь работает с фьючами как с **иммутабельными значениями**, т.е. передает (перемещает)
их в комбинаторы и терминаторы.

## `Result`

[`Result<T, E>`](source/exe/result/type/result.hpp) – это alias для [`std::expected<T, E>`](https://en.cppreference.com/w/cpp/utility/expected).

### `E`

Выбор типа для представления ошибки предоставляется пользователю фреймворка.

Хороший тип ошибки подразумевает возможность:
- добавлять к ошибке контекст (source location, correlation id и т.д.)
- комбинировать ошибки

### Конструкторы

#### `Ok`

Заголовок: [`result/make/ok.hpp`](source/exe/result/make/ok.hpp)

```cpp
auto r = result::Ok<int, std::error_code>(42);  // Значение 42
```

#### `Err`

Заголовок: [`result/make/err.hpp`](source/exe/result/make/err.hpp)

```cpp
auto r = result::Err<int, std::error_code>(
    std::make_error_code(std::errc::timed_out));
```

### Комбинаторы

```cpp
result::Ok<int, ExampleError>(1)
    | result::Map([](int v) {
        return v * 2;
      })
    | result::AndThen([](int v) {
        return result::Ok<int, ExampleError>(v + 1);
      })
    | result::OrElse([](ExampleError /*error*/) {  // Пропущен
        return result::Ok<int, ExampleError>(42);
      })
    | result::Map([](int v) {
        fmt::println("Ok.AndThen.OrElse -> {}", v);
      });
```

## Задание

1) Реализуйте `future::Contract` + `future::Get` с помощью wait-free рандеву и подписки с коллбэком
2) Прочитайте
   1) [Your Server as a Function](https://monkey.org/~marius/funsrv.pdf)
   2) [Futures aren't Ersatz Threads](https://monkey.org/~marius/futures-arent-ersatz-threads.html)
3) Реализуйте функциональные фьючи
   1) Конструкторы
   2) Последовательные комбинаторы
   3) Конкурентные комбинаторы
4) [Познакомьтесь с языком Haskell](https://www.haskell.org/tutorial/)
5) Подумайте над эффективной реализацией фьюч
6) Изучите каталоги / языки комбинаторов в разных PL / фреймворках:
   - [com.twitter.util.Future](https://twitter.github.io/util/docs/com/twitter/util/Future.html)
   - [Rust Futures](https://docs.rs/futures/latest/futures/)
   - [cats.effect.IO](https://typelevel.org/cats-effect/api/3.x/cats/effect/IO.html)
7) Составьте собственный язык (комбинаторов = программирования)!
### Дополнительно

#### Файберы

Файберы и фьючи должны хорошо комбинироваться:
- файберы лучше подходят для последовательной композиции,
- фьючи – для конкурентной.

Их комбинация должна сочетать сильные стороны обоих подходов.

Реализуйте функцию `fiber::Await` для ожидания произвольной фьючи в файбере.

Какие еще изменения вы внесете в фреймворк, где есть и файберы, и фьючи?

#### `First` & `All`

(Продвинутый уровень)

Напишите variadic версии комбинаторов `First` и `All` (обобщение `Both`).

#### `Unit`

Скройте от пользователя `Unit`-ы:

```cpp
auto f = future::Just() | future::Via(rt) | future::Map([] {
  // Маппер принимает значение типа Unit
  return 7;
});
```

## Реализация

Язык комбинаторов не имеет жесткой операционной семантики и дает свободу в выборе реализации.

Будет разумно начать с простой, и уже после подумать об эффективной.

Наша главная цель на данном этапе – познакомиться с функциональным подходом к concurrency и освоить
язык комбинаторов.

### Коллбэки

За исключением `Get`, в реализации фьюч нигде нет блокирующего ожидания!

Внутренняя механика всех комбинаторов и терминаторов – подписка на результат с помощью коллбэка.

Например, 
- в `Map` коллбэк планирует запуск маппера в установленном рантайме,
- в `Get` коллбэк устанавливает результат и будит ждущий поток,
- в `First` – "гоняется" с коллбэком другого "входа" за право выполнить выходную фьючу.

### Типы коллбэков

Разделим коллбэки на два класса:

1) В коллбэке исполняется **произвольный код пользователя** (например, в комбинаторах `Map` и `AndThen`)
2) В коллбэке исполняется **служебный код** комбинатора (например, `Flatten` или `First`) или терминатора (`Get` для потока или `Await` для файбера)

#### Пользователь

Потребуем, чтобы код пользователя всегда исполнялся в виде задач в рантайме.

#### Служебные коллбэки

Потребуем, чтобы служебные коллбэки всегда инлайнились (см. `Inline`).

### Синхронизация

Фьючам не требуется взаимное исключение, все терминаторы и комбинаторы в
задаче могут быть реализованы с гарантией прогресса wait-free.

## References

### Futures

- [Your Server as a Function](https://monkey.org/~marius/funsrv.pdf)
- [Futures aren't Ersatz Threads](https://monkey.org/~marius/futures-arent-ersatz-threads.html)
- [Асинхронность в программировании](https://habr.com/ru/companies/jugru/articles/446562/)

#### In the Wild

- [com.twitter.util.Future](https://twitter.github.io/util/docs/com/twitter/util/Future.html), [Twitter Futures](https://twitter.github.io/finagle/guide/Futures.html)
- [Rust Futures](https://docs.rs/futures/latest/futures/)
- [cats.effect.IO](https://typelevel.org/cats-effect/api/3.x/cats/effect/IO.html)
- [Futures for C++11 at Facebook](https://engineering.fb.com/2015/06/19/developer-tools/futures-for-c-11-at-facebook/), [Folly Futures](https://github.com/facebook/folly/blob/main/folly/docs/Futures.md)
- [YTsaurus Futures](https://github.com/ytsaurus/ytsaurus/tree/main/yt/yt/core/actions)    

### Errors

- [The Error Model](https://joeduffyblog.com/2016/02/07/the-error-model/)

### Haskell

- https://www.haskell.org/
- [A Gentle Introduction to Haskell](https://www.haskell.org/tutorial/)
- [The Evolution of a Haskell Programmer](https://people.willamette.edu/~fruehr/haskell/evolution.html)

### Linear types

- [Substructural type system](https://en.wikipedia.org/wiki/Substructural_type_system)
- [Austral's Linear Types](https://austral-lang.org/tutorial/linear-types)
- [Linear Haskell](https://arxiv.org/pdf/1710.09756.pdf)

### Monads

#### Introduction

- [About monads](https://www.haskell.org/tutorial/monads.html)
- [All About Monads](https://wiki.haskell.org/All_About_Monads)

#### Deep Dive

- [Monads for functional programming](https://homepages.inf.ed.ac.uk/wadler/papers/marktoberdorf/baastad.pdf) 
- [Imperative functional programming](https://www.microsoft.com/en-us/research/wp-content/uploads/1993/01/imperative.pdf)
- [Typeclassopedia](https://wiki.haskell.org/Typeclassopedia)
- [A monad is just a monoid in the category of endofunctors, what's the problem?](https://stackoverflow.com/questions/3870088/a-monad-is-just-a-monoid-in-the-category-of-endofunctors-whats-the-problem)
- [Functors, Applicatives, And Monads In Pictures](https://www.adit.io/posts/2013-04-17-functors,_applicatives,_and_monads_in_pictures.html)
- [Abstraction, intuition, and the “monad tutorial fallacy”](https://byorgey.wordpress.com/2009/01/12/abstraction-intuition-and-the-monad-tutorial-fallacy/)

