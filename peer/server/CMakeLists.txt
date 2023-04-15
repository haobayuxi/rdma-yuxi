

set(SERVER_SOURCE server.cc)
add_executable(server ${SERVER_SOURCE})
target_link_libraries(server yuxi rlib)
