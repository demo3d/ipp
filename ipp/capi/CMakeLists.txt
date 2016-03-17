cmake_minimum_required(VERSION 3.2)


file(GLOB_RECURSE LIBIPP_CAPI_SOURCES "**.cpp" "**.hpp")


if(CMAKE_SYSTEM_NAME MATCHES "Emscripten")
    # emscripten will compile library targets -> llvm bitcode but will compile
    # executable to js module and won't complain about no "main" function so use executable target
    add_executable(ipp_capi ${LIBIPP_CAPI_SOURCES})
    set_target_properties(ipp_capi
        PROPERTIES
        COMPILE_FLAGS "-std=c++14"
        LINK_FLAGS
            "-s DISABLE_EXCEPTION_CATCHING=0 \
             -s RESERVED_FUNCTION_POINTERS=32 \
             --pre-js ${CMAKE_CURRENT_LIST_DIR}/emscripten/prefix.js \
             --post-js ${CMAKE_CURRENT_LIST_DIR}/emscripten/postfix.js")
else()
    add_library(ipp_capi SHARED ${LIBIPP_CAPI_SOURCES})
    set_target_properties(ipp_capi
        PROPERTIES
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED ON)
endif()

target_link_libraries(ipp_capi ipp)
