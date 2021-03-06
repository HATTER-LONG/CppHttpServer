find_package(Catch2 REQUIRED CONFIG)

FILE(GLOB TESTSRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cxx"
    "${CMAKE_SOURCE_DIR}/Src/ConfigControlImp.cpp"
    "${CMAKE_SOURCE_DIR}/Src/HttpServer.cpp")

add_definitions(-DCONFIGFILE_FULLPATH="${CMAKE_CURRENT_SOURCE_DIR}/")

list(APPEND LOCAL_LINK_LIB Catch2::Catch2WithMain)

set(PROJECTNAME CppHttpServerTest)

add_executable(${PROJECTNAME} ${TESTSRC})

target_link_libraries(${PROJECTNAME} ${LOCAL_LINK_LIB} -levent -levent_pthreads)