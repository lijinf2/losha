include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/gqr/include)


SET(TOOLS
    sample_base
)

FOREACH(APP ${TOOLS})
ADD_EXECUTABLE(${APP} ${APP}.cpp)
ENDFOREACH(APP)
