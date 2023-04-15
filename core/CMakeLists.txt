# Author: Ming Zhang
# Copyright (c) 2022

set(CONNECTION_SRC
        connection/qp_manager.cc
        )

set(SCHEDULER_SRC
        scheduler/corotine_scheduler.cc
        )

add_library(yuxi STATIC
        ${CONNECTION_SRC}
        # ${SCHEDULER_SRC}
        )

set_target_properties(yuxi PROPERTIES LINKER_LANGUAGE CXX)

target_link_libraries(yuxi rlib pthread boost_coroutine boost_context boost_system jsoncpp)