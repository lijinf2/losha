include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(simhash simhash.cpp )
# target_link_libraries(plsh ${HUSKY_EXTERNAL_LIB})
target_link_libraries(simhash ${losha})
