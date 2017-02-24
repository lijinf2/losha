include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(PLSH plsh.cpp )
# target_link_libraries(PLSH ${HUSKY_EXTERNAL_LIB})
target_link_libraries(PLSH ${losha})
