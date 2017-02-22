# to use headers in other subdirectories
include_directories(${PROJECT_SOURCE_DIR})

# # build static library
# add_library(lshcore-sparse-obj OBJECT
#    sparsecoslshfactory.hpp
#    apsparsecoslshfactory.hpp)
#
# add_library(lshsparse STATIC $<TARGET_OBJECTS:lshcore-sparse-obj>)
# SET_TARGET_PROPERTIES(lshsparse PROPERTIES LINKER_LANGUAGE CXX)
