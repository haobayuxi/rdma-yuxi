# Author: Ming Zhang
# Copyright (c) 2022

set(CONNECTION_SRC
        connection/rdma.cc
        connection/rdma_transport.cc
        )

set(SCHEDULER_SRC
        scheduler/corotine_scheduler.cc
        )

add_library(yuxi STATIC
        ${CONNECTION_SRC}
        ${SCHEDULER_SRC}
        )

set_target_properties(yuxi PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(yuxi rlib ibverbs pthread boost_coroutine boost_context boost_system jsoncpp)