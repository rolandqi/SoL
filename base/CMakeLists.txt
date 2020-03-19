set(base_SRCS
  Thread.cpp 
  Condition.cpp 
  async_logging.cpp 
  log_file.cpp
  log_stream.cpp
  logging.cpp
  file_util.cpp 
  process_info.cpp 
  timestamp.cpp
  countdown_latch.cpp 
  timezone.cpp
  date.cpp
  )

add_library(base ${base_SRCS})
target_link_libraries(base pthread rt)

install(TARGETS base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/base)