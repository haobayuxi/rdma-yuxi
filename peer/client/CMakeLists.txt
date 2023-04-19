
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -libverbs -std=c++11")

set(SERVER_SOURCE client.cc)
add_executable(client ${SERVER_SOURCE})
target_link_libraries(client yuxi)
