add_executable(pingpong_client client.cc)
target_link_libraries(pingpong_client net)

add_executable(pingpong_server server.cc)
target_link_libraries(pingpong_server net)

add_executable(pingpong_bench bench.cc)
target_link_libraries(pingpong_bench net)

