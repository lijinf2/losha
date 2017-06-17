include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/husky)
include_directories(${HUSKY_EXTERNAL_INCLUDE})

ADD_EXECUTABLE(SCLF sparsecoslshfactory_test.cpp)
TARGET_LINK_LIBRARIES(SCLF ${losha})

ADD_EXECUTABLE(topkpairs_test topkpairs_test.cpp)
TARGET_LINK_LIBRARIES(topkpairs_test ${losha})
