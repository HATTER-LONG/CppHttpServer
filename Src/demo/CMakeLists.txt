FILE(GLOB PROSRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/democlient.cpp" 
)

set(PROJECTNAME demoCppHttpClient)
add_executable(${PROJECTNAME} ${PROSRC})

target_link_libraries(${PROJECTNAME} ${LOCAL_LINK_LIB} -levent -levent_pthreads)