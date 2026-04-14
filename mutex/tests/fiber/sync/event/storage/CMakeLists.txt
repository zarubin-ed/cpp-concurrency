add_executable(exe_tests_fiber_sync_event_storage_random
        random.cpp)
target_link_libraries(exe_tests_fiber_sync_event_storage_random exe course)

add_executable(exe_tests_fiber_sync_event_storage_stress
        stress.cpp)
target_link_libraries(exe_tests_fiber_sync_event_storage_stress exe course)
