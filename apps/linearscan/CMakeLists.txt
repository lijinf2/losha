include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(linearscan linearscan.cpp )
target_link_libraries(linearscan ${losha})
