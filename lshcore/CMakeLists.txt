add_library(lshcore-obj OBJECT 
    densevector.hpp
    lshutils.hpp
    lshfactory.hpp
    lshengine.hpp
    lshquery.hpp
    lshitem.hpp)

add_subdirectory(sparse)
add_library(lshcore-libs STATIC $<TARGET_OBJECTS:lshcore-obj> $<TARGET_OBJECTS:lshcore-sparse-obj>)
SET_TARGET_PROPERTIES(lshcore-libs PROPERTIES LINKER_LANGUAGE CXX)
