
set(DB_SOURCES micro_db.cc)

add_library(micro_db STATIC ${DB_SOURCES})

set_target_properties(micro_db PROPERTIES LINKER_LANGUAGE CXX)

# target_link_libraries(smallbank_txn ford)