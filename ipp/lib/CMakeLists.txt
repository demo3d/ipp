cmake_minimum_required(VERSION 3.2)
project(ipp_lib)


add_definitions(-DLODEPNG_NO_COMPILE_ENCODER -DLODEPNG_NO_COMPILE_DISK -DLODEPNG_NO_COMPILE_ERROR_TEXT)
if (CMAKE_BUILD_TYPE MATCHES RELEASE)
add_definitions(-DLODEPNG_NO_COMPILE_ERROR_TEXT)
endif()


file(GLOB_RECURSE LIBIPP_SOURCES "src/**.cpp" "src/**.hpp" "include/**.hpp")
add_library(ipp ${LIBIPP_SOURCES})


if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
else()
    set_property(TARGET ipp PROPERTY CXX_STANDARD 14)
    set_property(TARGET ipp PROPERTY CXX_STANDARD_REQUIRED ON)
endif()


target_include_directories(ipp PUBLIC
   "${CMAKE_CURRENT_LIST_DIR}/include"
   "${CMAKE_SOURCE_DIR}/../build/schema/cpp"
    ${GLM_INCLUDE_DIRS}
    ${FLATBUFFERS_INCLUDE_DIRS}
    ${JSON_INCLUDE_DIRS})

if(NOT IVL_LOGGING_DISABLED)
    target_include_directories(ipp PUBLIC ${CPPFORMAT_INCLUDE_DIRS})
endif()

# don't build tests for Emscripten
if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten" AND NOT IVL_LOGGING_DISABLED)
    file(GLOB_RECURSE TEST_LIBIPP_SOURCES "test/**.cpp")
    add_executable(test_ipp ${TEST_LIBIPP_SOURCES})
    target_include_directories(test_ipp PRIVATE ${CATCH_INCLUDE_DIRS})
    target_link_libraries(test_ipp ipp)

    set_target_properties(test_ipp
        PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON)
    target_link_libraries(test_ipp GLESv2 glfw)
endif()
