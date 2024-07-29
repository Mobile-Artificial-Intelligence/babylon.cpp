cmake_minimum_required(VERSION 3.18)

project(
    babylon
    VERSION 0.0.1
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
    src/phonemizer.cpp
    src/voice.cpp
)

# Set the output directory for the shared library
set_target_properties(
    babylon
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${BABYLON_LIB_INSTALL_DIR}
)

set(onnxruntime_BUILD_SHARED_LIB ON)
set(onnxruntime_BUILD_UNIT_TESTS OFF)

# Set ONNX Runtime source directory
set(ONNXRUNTIME_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/onnxruntime)

# Add ONNX Runtime subdirectory
add_subdirectory(${ONNXRUNTIME_SOURCE_DIR}/cmake)

# Include ONNX Runtime headers
target_include_directories(
    babylon
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ONNXRUNTIME_SOURCE_DIR}/include
)

# Link ONNX Runtime to babylon
target_link_libraries(
    babylon
    onnxruntime
)

# Link against CoreFoundation and Foundation on macOS
if(APPLE)
    target_link_libraries(
        babylon
        "-framework CoreFoundation"
        "-framework Foundation"
    )
endif()

# Include example directory if EXAMPLES flag is set
option(BUILD_EXAMPLES "Build examples" OFF)

if(BUILD_EXAMPLES)
    add_subdirectory(example)
endif()
