
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(SERVER_SOURCE 
    server.cc)
add_executable(server ${SERVER_SOURCE})
target_link_libraries(server jsoncpp erpc)
