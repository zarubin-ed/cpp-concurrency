begin_task()

add_task_library(exe)

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

# Future

add_task_test(future_unit_tests tests/future/unit.cpp)
add_task_test(future_stress_tests tests/future/stress.cpp)
add_task_test(future_model_tests tests/future/model.cpp)

end_task()
