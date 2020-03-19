include(CheckFunctionExists)

check_function_exists(accept4 HAVE_ACCEPT4)
if(NOT HAVE_ACCEPT4)
  set_source_files_properties(SocketsOps.cc PROPERTIES COMPILE_FLAGS "-DNO_ACCEPT4")
endif()

set(net_SRCS
  Util.cpp
  Channel.cpp 
  Epoll.cpp
  EventLoop.cpp
  Timer.cpp
  EventLoopThread.cpp
  EventLoopThreadPool.cpp
  Server.cpp
  Client.cpp
  )

add_library(net ${net_SRCS})
target_link_libraries(net base)


install(TARGETS net DESTINATION lib)

set(HEADERS
  Util.h
  Channel.h
  Epoll.h
  EventLoop.h
  Timer.h
  EventLoopThread.h
  EventLoopThreadPool.h
  Server.h
  Client.h
  )
install(FILES ${HEADERS} DESTINATION include/net)