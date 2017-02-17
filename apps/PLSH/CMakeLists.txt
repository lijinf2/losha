include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

### applications
set(husky husky-lib husky-io husky-core husky-base)
add_executable(PLSH plsh.cpp )
target_link_libraries(PLSH ${husky})
target_link_libraries(PLSH ${HUSKY_EXTERNAL_LIB})
target_link_libraries(PLSH ${losha})

### test
set(husky husky-io husky-core husky-lib)
add_executable(TestPLSH testplsh.cpp )
target_link_libraries(TestPLSH ${husky})
target_link_libraries(TestPLSH ${HUSKY_EXTERNAL_LIB})
