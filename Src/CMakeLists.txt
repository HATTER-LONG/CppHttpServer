FILE(GLOB PROSRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
    "${CMAKE_SOURCE_DIR}/Src/*.cpp")

add_definitions(-DCONFIGFILE_FULLPATH="${CMAKE_CURRENT_SOURCE_DIR}/")
set(PROJECTNAME CppHttpServer)
add_executable(${PROJECTNAME} ${PROSRC})

target_link_libraries(${PROJECTNAME} ${LOCAL_LINK_LIB} -levent)