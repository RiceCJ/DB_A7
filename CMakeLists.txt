cmake_minimum_required(VERSION 3.6)
project(dbms_test)

set(CMAKE_CXX_STANDARD 11)

include_directories(BufferMgr/headers Catalog/headers DatabaseTable/headers Qunit/headers Record/headers SQL/headers RelOps/headers)

file(GLOB COMMON_HEADERS "BufferMgr/headers/*.h" "Catalog/headers/*.h" "DatabaseTable/headers/*.h" "Qunit/headers/*.h" "Record/headers/*.h" "SQL/headers/*.h" "RelOps/headers/*.h")
file(GLOB COMMON_SOURCE "BufferMgr/source/*.cc" "Catalog/source/*.cc" "DatabaseTable/source/*.cc" "Record/source/*.cc" "SQL/source/*.c*" "SQL/source/*.c" "RelOps/source/*.cc")

set(SOURCE_FILES_BUFFER_TEST ${COMMON_HEADERS} ${COMMON_SOURCE} BufferTest/source/BufferQUnit.cc)
add_executable(buffer_test ${SOURCE_FILES_BUFFER_TEST})

set(SOURCE_FILES_RECORD_TEST ${COMMON_HEADERS} ${COMMON_SOURCE} RecordTest/source/RecordQUnit.cc)
add_executable(record_test ${SOURCE_FILES_RECORD_TEST})

set(SOURCE_FILES_SORT_TEST ${COMMON_HEADERS} ${COMMON_SOURCE} SortTest/source/SortQUnit.cc)
add_executable(sort_test ${SOURCE_FILES_SORT_TEST})

set(SOURCE_FILES_BPLUS_TEST ${COMMON_HEADERS} ${COMMON_SOURCE} BPlusTest/source/BPlusQUnit.cc)
add_executable(bplus_test ${SOURCE_FILES_BPLUS_TEST})

set(SOURCE_FILES_SQL_TEST ${COMMON_HEADERS} ${COMMON_SOURCE} SQLTest/source/main.cc)
add_executable(sql_test ${SOURCE_FILES_SQL_TEST})

set(SOURCE_FILES_REL_OP_TEST ${COMMON_HEADERS} ${COMMON_SOURCE} RelOpTest/source/RelOpQUnit.cc)
add_executable(rel_op_test ${SOURCE_FILES_REL_OP_TEST})

add_custom_target(
        create_lexer
        COMMAND flex -t ${CMAKE_CURRENT_SOURCE_DIR}/SQL/source/Lexer.l > ${CMAKE_CURRENT_SOURCE_DIR}/SQL/source/Lexer.c
)
add_dependencies(sql_test create_lexer)

add_custom_target(
        create_parser
        COMMAND bison -d -o ${CMAKE_CURRENT_SOURCE_DIR}/SQL/source/Parser.c ${CMAKE_CURRENT_SOURCE_DIR}/SQL/source/Parser.y
)
add_dependencies(sql_test create_parser)