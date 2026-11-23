[English](README.md) | [Русский](README.ru.md)

# Concurrent and Asynchronous Programming in C++

An educational C++ framework for concurrent and asynchronous programming, featuring fibers, futures, schedulers, timers, and synchronization primitives.

## Project Structure

The project is organized into several subsystems:

- **`runtime`** — task execution, scheduling, and thread pools.
- **`fiber`** — lightweight concurrency built on stackful coroutines.
- **`future`** — asynchronous abstractions and functional combinators.
- **`result`** — value/error representation and composition.
- **`thread`** — thread synchronization primitives.
- **`gc`** — memory reclamation mechanisms.

```text
source/
├── fiber/                   # Stackful coroutines and fiber primitives
│   ├── chan/                # Buffered and rendezvous channels
│   ├── core/                # Coroutine internals, handles, states, callbacks
│   ├── sched/               # Fiber scheduling: Go, Yield, SleepFor, Suspend
│   └── sync/                # Fiber mutexes, events, wait queues
│
├── future/                  # Futures, promises, functional combinators
│   ├── combine/             # Both, First, All, FirstOk
│   ├── detail/              # Internal implementation utilities
│   ├── make/                # Contract, Value, Failure, Submit, etc.
│   ├── syntax/              # Pipe syntax and adapters
│   ├── terminate/           # Waiting and result retrieval
│   ├── trait/               # Type traits and metaprogramming
│   └── type/                # Core future types
│
├── result/                  # Value/error representation and operations
│   ├── combine/             # Result composition and transformations
│   ├── make/                # Success and failure constructors
│   ├── syntax/              # Syntactic adapters
│   └── trait/               # Result type traits
│
├── runtime/                 # Task execution infrastructure
│   ├── detail/              # Shared runtime internals
│   ├── multi_thread/        # Thread pools, work stealing, timer thread
│   │   ├── v1               # Thread pool with a global task queue
│   │   └── v2               # Work-stealing thread pool
│   ├── sandbox/             # Single-threaded runtime for deterministic testing
│   ├── task/                # Intrusive tasks, boxed tasks, task interfaces
│   ├── timer/               # Timers and delayed task scheduling
│   ├── view/                # Lightweight runtime views and handles
│   ├── multi_thread.hpp     # Public multithreaded runtime interface
│   └── sandbox.hpp          # Public sandbox runtime interface
│
├── thread/
│   ├── condvar.hpp          # Condition variable
│   ├── mutex.hpp            # Mutex
│   ├── primitives.hpp       # Common threading primitives
│   ├── spinlock.hpp         # Spinlock
│   ├── unique_lock.hpp      # Unique lock
│   └── wait_group.hpp       # Wait group
│
└── gc/                      # Memory reclamation
    ├── hazard
    └── atomic_shared_ptr
```

## Overview

This project is an educational C++ framework exploring the implementation of concurrency and asynchronous programming primitives.

At its core is a custom task execution runtime with two multithreaded implementations:

- **Global-queue thread pool (v1):** A basic thread pool where workers share a single task queue.
- **Work-stealing thread pool (v2):** A more advanced scheduler where workers maintain local task queues and steal tasks from one another to improve load balancing.

The runtime also includes a single-threaded sandbox for testing and simulating time, an intrusive task system, and timers for delayed task scheduling.

On top of the runtime, the framework implements **lightweight concurrency using stackful fibers**. This includes fiber scheduling operations (`Go`, `Yield`, `SleepFor`, `Suspend`), buffered and rendezvous channels, and synchronization primitives such as mutexes, events, and wait queues.

The project also provides a **composable futures and promises library**, featuring pipe-based syntax and combinators such as `Both`, `First`, `All`, and `FirstOk`. These abstractions enable asynchronous computations to be composed in a declarative style.

Additional components include a result abstraction for composing successful and failed computations, basic thread synchronization primitives, and memory reclamation mechanisms.

---

**Project context:** I developed this project as part of a [Concurrent Programming course](https://gitlab.com/Lipovsky/concurrency-course) at the Moscow Institute of Physics and Technology (MIPT).
