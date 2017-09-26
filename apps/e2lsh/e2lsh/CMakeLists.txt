include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

### applications
set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(e2lsh e2lsh.cpp )
target_link_libraries(e2lsh ${losha})
