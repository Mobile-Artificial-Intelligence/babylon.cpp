cmake_minimum_required(VERSION 3.18)

project(
    babylon
    VERSION 0.0.2
    DESCRIPTION "Babylon Neural Text To Speech Engine"
    HOMEPAGE_URL "https://github.com/danemadsen/babylon.cpp"
    LANGUAGES CXX
)

set(BABYLON_LIB_INSTALL_DIR ${CMAKE_CURRENT_LIST_DIR}/lib CACHE PATH "Babylon library install directory")

# Enable C++17 standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler options to support UTF-8 encoding
if(MSVC)
    add_compile_options("/utf-8")
else()
    add_compile_options("-finput-charset=UTF-8" "-fexec-charset=UTF-8")
endif()

# Add the babylon library
add_library(
    babylon
    SHARED
    src/babylon.cpp
    src/cleaners.cpp
    src/phonemizer.cpp
    src/voice.cpp
)

if(NOT APPLE)
    target_link_options(babylon PRIVATE LINKER:--build-id=none)
    target_compile_options(babylon PRIVATE "-ffile-prefix-map=${CMAKE_CURRENT_SOURCE_DIR}=babylon")
endif()

# Set the output directory for the shared library
set_target_properties(
    babylon
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BABYLON_LIB_INSTALL_DIR}
)

# if linux or windows
if(NOT APPLE AND NOT ANDROID)
    set_target_properties(
        babylon
        PROPERTIES
        BUILD_WITH_INSTALL_RPATH TRUE
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Set ONNX Runtime source directory
set(ONNXRUNTIME_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/onnxruntime)

if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)
    set(ONNX_USE_MSVC_STATIC_RUNTIME ON)
    set(protobuf_MSVC_STATIC_RUNTIME ON)
endif()

# Set ONNX Runtime options
set(onnxruntime_BUILD_SHARED_LIB ON)
set(onnxruntime_BUILD_UNIT_TESTS OFF)   

# Add ONNX Runtime subdirectory
set(FETCHCONTENT_SOURCE_DIR_EIGEN "/tmp/eigen")
set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)
add_subdirectory(${ONNXRUNTIME_SOURCE_DIR}/cmake)   

# Set the output directory for the shared library
set_target_properties(
    onnxruntime
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BABYLON_LIB_INSTALL_DIR}
)

# Include ONNX Runtime headers
target_include_directories(
    babylon
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ONNXRUNTIME_SOURCE_DIR}/include/onnxruntime/core/session
)

target_link_directories(
    babylon PUBLIC
    ${BABYLON_LIB_INSTALL_DIR}
)
if(WIN32)
    target_link_libraries(
        babylon
        onnxruntime.lib
    )
elseif(APPLE)
    target_link_libraries(
        babylon
        onnxruntime
        "-framework CoreFoundation"
        "-framework Foundation"
    )
else()
    target_link_libraries(
        babylon
        onnxruntime
    )
endif()

# Include example directory if EXAMPLES flag is set
option(BUILD_EXAMPLES "Build examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
