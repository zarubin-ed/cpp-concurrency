begin_task()

# Library
add_task_library(exe)

# Playground
add_task_playground(play)

# Tests

# Thread

## WaitGroup

add_task_test(thread_wait_group_unit_tests tests/thread/wait_group/unit.cpp)
add_task_test(thread_wait_group_random_tests tests/thread/wait_group/random.cpp)
add_task_test(thread_wait_group_futex_tests tests/thread/wait_group/futex.cpp)
add_task_test(thread_wait_group_model_tests tests/thread/wait_group/model.cpp)
add_task_test(thread_wait_group_storage_tests tests/thread/wait_group/storage.cpp)

# Runtime

## Sandbox

add_task_test(runtime_sandbox_tasks_unit_tests tests/runtime/sandbox/tasks.cpp)
add_task_test(runtime_sandbox_timers_unit_tests tests/runtime/sandbox/timers.cpp)

## MultiThread

### Tasks

add_task_test(runtime_multi_thread_tasks_unit_tests tests/runtime/multi_thread/tasks/unit.cpp)
add_task_test(runtime_multi_thread_tasks_random_tests tests/runtime/multi_thread/tasks/random.cpp)
add_task_test(runtime_multi_thread_tasks_stress_tests tests/runtime/multi_thread/tasks/stress.cpp)

### Timers

add_task_test(runtime_multi_thread_timers_unit_tests tests/runtime/multi_thread/timers/unit.cpp)
add_task_test(runtime_multi_thread_timers_random_tests tests/runtime/multi_thread/timers/random.cpp)
add_task_test(runtime_multi_thread_timers_stress_tests tests/runtime/multi_thread/timers/stress.cpp)

# Fibers

## Sched

### Yield

add_task_test(fiber_sched_yield_unit_tests tests/fiber/sched/yield/unit.cpp)
add_task_test(fiber_sched_yield_stress_tests tests/fiber/sched/yield/stress.cpp)

### SleepFor

add_task_test(fiber_sched_sleep_for_unit_tests tests/fiber/sched/sleep_for/unit.cpp)
add_task_test(fiber_sched_sleep_for_stress_tests tests/fiber/sched/sleep_for/stress.cpp)

## Sync

### Event

add_task_test(fiber_sync_event_unit_tests tests/fiber/sync/event/unit.cpp)
add_task_test(fiber_sync_event_random_tests tests/fiber/sync/event/random.cpp)
add_task_test(fiber_sync_event_stress_tests tests/fiber/sync/event/stress.cpp)

add_task_test(fiber_sync_event_storage_random_tests tests/fiber/sync/event/storage/random.cpp)
add_task_test(fiber_sync_event_storage_stress_tests tests/fiber/sync/event/storage/stress.cpp)

### Mutex

add_task_test(fiber_sync_mutex_unit_tests tests/fiber/sync/mutex/unit.cpp)
add_task_test(fiber_sync_mutex_random_tests tests/fiber/sync/mutex/random.cpp)
add_task_test(fiber_sync_mutex_stress_tests tests/fiber/sync/mutex/stress.cpp)
add_task_test(fiber_sync_mutex_fork_tests tests/fiber/sync/mutex/fork.cpp)
add_task_test(fiber_sync_mutex_stack_overflow_tests tests/fiber/sync/mutex/stack_overflow.cpp)

### WaitGroup

add_task_test(fiber_sync_wait_group_unit_tests tests/fiber/sync/wait_group/unit.cpp)
add_task_test(fiber_sync_wait_group_random_tests tests/fiber/sync/wait_group/random.cpp)
add_task_test(fiber_sync_wait_group_stress_tests tests/fiber/sync/wait_group/stress.cpp)

add_task_test(fiber_sync_wait_group_storage_random_tests tests/fiber/sync/wait_group/storage/random.cpp)
add_task_test(fiber_sync_wait_group_storage_stress_tests tests/fiber/sync/wait_group/storage/stress.cpp)

end_task()
