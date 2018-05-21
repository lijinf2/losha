include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/gqr/include)

link_libraries(-lpthread)
set(EXTERNAL_LIB ${CMAKE_THREAD_LIBS_INIT})

SET(TOOLS
    evaluate_triplets
    cal_groundtruth_idfvecs
)

FOREACH(APP ${TOOLS})
ADD_EXECUTABLE(${APP} ${APP}.cpp)
ENDFOREACH(APP)
