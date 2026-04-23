# Конкурентное и асинхронное программирование на С++

Учебный C++ framework для конкурентного программирования: fibers, futures, schedulers, timers и примитивы синхронизации.


## Структура проекта

Проект разделён на несколько основных подсистем. `runtime` отвечает за исполнение задач и планирование, `fiber` реализует lightweight concurrency на основе стековых корутин, `future` предоставляет асинхронные абстракции и функциональные combinators, `result` отвечает за представление успешных и ошибочных вычислений, а `thread` содержит базовые потоковые примитивы синхронизации.

```text
source/
├── fiber/                   # стековые корутины и примитивы поверх них
│   ├── chan/                # каналы: buffered / rendezvous 
│   ├── core/                # внутреннее устройство fiber: coroutine, handle, state, callbacks
│   ├── sched/               # запуск и перепланирование fiber: Go, Yield, SleepFor, Suspend
│   └── sync/                # примитивы синхронизации для fiber: mutex, event, wait_queue
│
├── future/                  # future/promise и функциональные combinators
│   ├── combine/             # комбинаторы Both, First, All, FirstOk
│   ├── detail/              # внутренние вспомогательные реализации
│   ├── make/                # конструкторы futures: Contract, Value, Failure, Submit и т.п.
│   ├── syntax/              # pipe-синтаксис и адаптеры для удобной записи
│   ├── terminate/           # завершающие операции: ожидание, получение результата
│   ├── trait/               # type traits и метафункции для futures
│   └── type/                # базовые типы библиотеки future
│
├── result/                  # тип результата вычисления (value/error) и операции над ним
│   ├── combine/             # композиции и преобразования result-типов
│   ├── make/                # конструкторы успешных и ошибочных результатов
│   ├── syntax/              # синтаксические адаптеры и sugar
│   └── trait/               # traits для result-типов
│
├── runtime/                 # инфраструктура выполнения задач
│   ├── detail/              # общие внутренние детали runtime
│   ├── multi_thread/        # многопоточный runtime: thread pool, work stealing, timer thread
│   │   ├── v1               # thread pool с одной глобальной очередью задач
│   │   └── v2               # work-stealing thread pool 
│   ├── sandbox/             # однопоточный/manual runtime для тестов и моделирования
│   ├── task/                # intrusive tasks, boxed tasks, интерфейсы задач
│   ├── timer/               # таймеры, delay scheduling, очереди отложенных задач
│   ├── view/                # lightweight view/handle на runtime
│   ├── multi_thread.hpp     # публичный интерфейс multithread runtime
│   └── sandbox.hpp          # публичный интерфейс sandbox runtime
└── thread/
    ├── condvar.hpp              # condition variable
    ├── mutex.hpp                # mutex
    ├── primitives.hpp           # общие thread primitives
    ├── spinlock.hpp             # spinlock
    ├── unique_lock.hpp          # unique_lock
    └── wait_group.hpp           # wait group
```
# Описание
Проект представляет собой учебный C++ framework для конкурентного и асинхронного программирования. В его основе лежит собственный runtime с двумя многопоточными реализациями: базовым thread pool с глобальной очередью задач и более продвинутым work-stealing thread pool, в котором worker’ы используют локальные очереди и воруют задачи друг у друга для лучшей балансировки нагрузки. Отдельно реализованы sandbox runtime для тестов и моделирования времени, система intrusive tasks и таймеры для отложенного планирования.

Поверх runtime построена модель lightweight concurrency на fibers: стековые корутины, операции планирования (Go, Yield, SleepFor, Suspend), каналы (buffered и rendezvous) и примитивы синхронизации, такие как mutex, event и wait_queue. В проекте также есть функциональная библиотека future/promise с pipe-синтаксисом и комбинаторами вроде Both, First, All и FirstOk, позволяющими собирать асинхронные вычисления в декларативном стиле. Дополнительно реализован слой result для value/error composition и набор базовых потоковых примитивов.

> Проект был выполнен мной в рамках курса по конкурентному программированию, который я проходил в [МФТИ](https://gitlab.com/Lipovsky/concurrency-course).
