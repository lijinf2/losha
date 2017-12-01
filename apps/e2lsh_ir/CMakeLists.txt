include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

### applications
set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(e2lsh_ir e2lsh_ir.cpp )
target_link_libraries(e2lsh_ir ${losha})
