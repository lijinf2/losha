include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)

### applications
set(losha husky losha-lib ${HUSKY_EXTERNAL_LIB})
add_executable(knngraph_train knngraph_train.cpp )
target_link_libraries(knngraph_train ${losha})

add_executable(knnconnected_components knnconnected_components.cpp )
target_link_libraries(knnconnected_components ${losha})
