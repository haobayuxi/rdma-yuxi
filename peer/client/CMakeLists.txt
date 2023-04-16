

set(SERVER_SOURCE client.cc)
add_executable(client ${SERVER_SOURCE})
target_link_libraries(client yuxi rlib)
