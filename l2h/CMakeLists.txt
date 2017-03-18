INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/husky)

set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(dish dish.cpp)
target_link_libraries(dish ${losha})
