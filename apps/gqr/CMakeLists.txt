include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

### applications
set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(gqr gqr.cpp )
target_link_libraries(gqr ${losha})
