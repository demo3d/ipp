cmake_minimum_required (VERSION 3.2)

file(GLOB_RECURSE DEVSHELL_SOURCES "src/**.cpp" "include/**.hpp")
add_executable(ipp_devshell ${DEVSHELL_SOURCES})
target_include_directories(ipp_devshell PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_link_libraries(ipp_devshell ipp GLESv2 glfw)
set_target_properties(ipp_devshell
    PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON)
