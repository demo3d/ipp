cmake_minimum_required(VERSION 3.2)
project(ipp)


option(IVL_LOGGING_DISABLED "Disable log messages in binaries trough IVL_LOG and IVL_LOG_THROW_ERROR." OFF)


set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})


set(CPPFORMAT_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/extern/cppformat" CACHE PATH "")
set(CATCH_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/extern/Catch/single_include" CACHE PATH "")
set(GLM_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/extern/glm" CACHE PATH "" FORCE)
set(FLATBUFFERS_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/extern/flatbuffers/include" CACHE PATH "")
set(JSON_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/extern/json/src" CACHE PATH "")


set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall")


if(IVL_LOGGING_DISABLED)
    add_definitions(-DIVL_LOGGING_DISABLED)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DIVL_DEBUG_BUILD)
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    # extra debugging information for all built libraries under emscripten debug build
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g4 -s EXCEPTION_DEBUG=1 -s DEMANGLE_SUPPORT=1")
    # remove all logging info in Release and don't link against cppformat to minimize js size
endif()

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

add_subdirectory(lib)
add_subdirectory(capi)

# don't build devshell for emscripten
if(NOT CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    add_subdirectory(devshell)
endif()
