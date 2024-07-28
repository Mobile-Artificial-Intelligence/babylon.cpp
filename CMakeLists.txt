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

# Look for onnxruntime files in <root>/lib
if(NOT DEFINED ONNXRUNTIME_DIR)
    set(ONNXRUNTIME_VERSION "1.18.1")

    if(WIN32)
        # Windows x86-64
        set(ONNXRUNTIME_PREFIX "onnxruntime-win-x64-static_lib-${ONNXRUNTIME_VERSION}")
        set(ONNXRUNTIME_EXT "zip")
    elseif (APPLE)
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
            # MacOS x86-64
            set(ONNXRUNTIME_PREFIX "onnxruntime-osx-x86_64-static_lib-${ONNXRUNTIME_VERSION}")
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL arm64)
            # MacOS Apple Silicon
            set(ONNXRUNTIME_PREFIX "onnxruntime-osx-arm64-static_lib-${ONNXRUNTIME_VERSION}")
        else()
            message(FATAL_ERROR "Unsupported architecture for onnxruntime")
        endif()

        set(ONNXRUNTIME_EXT "tgz")
    else()
        if(CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
            # Linux x86-64
            set(ONNXRUNTIME_PREFIX "onnxruntime-linux-x64-static_lib-${ONNXRUNTIME_VERSION}")
        elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL aarch64)
            # Linux ARM 64-bit
            set(ONNXRUNTIME_PREFIX "onnxruntime-linux-aarch64-static_lib-${ONNXRUNTIME_VERSION}")
        else()
            message(FATAL_ERROR "Unsupported architecture for onnxruntime")
        endif()

        set(ONNXRUNTIME_EXT "tgz")
    endif()

    if(NOT DEFINED ONNXRUNTIME_URL)
        set(ONNXRUNTIME_URL "https://github.com/supertone-inc/onnxruntime-build/releases/download/v${ONNXRUNTIME_VERSION}/${ONNXRUNTIME_PREFIX}.${ONNXRUNTIME_EXT}")
    endif()

    set(DOWNLOAD_DIR "${CMAKE_CURRENT_LIST_DIR}/download")

    set(ONNXRUNTIME_FILENAME "${DOWNLOAD_DIR}/${ONNXRUNTIME_PREFIX}.${ONNXRUNTIME_EXT}")
    set(ONNXRUNTIME_DIR "${DOWNLOAD_DIR}/${ONNXRUNTIME_PREFIX}")

    if(NOT EXISTS "${ONNXRUNTIME_DIR}")
        if(NOT EXISTS "${ONNXRUNTIME_FILENAME}")
            # Download onnxruntime release
            message("Downloading ${ONNXRUNTIME_URL}")
            file(DOWNLOAD "${ONNXRUNTIME_URL}" "${ONNXRUNTIME_FILENAME}")
        endif()

        # Extract .zip or .tgz to a directory like lib/onnxruntime-linux-x64-1.18.1/
        file(ARCHIVE_EXTRACT INPUT "${ONNXRUNTIME_FILENAME}" DESTINATION "${DOWNLOAD_DIR}")
    endif()
endif()

# set include directories
target_include_directories(
    babylon
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ONNXRUNTIME_DIR}/include
)

add_library(onnxruntime STATIC IMPORTED)

# Import options for onnxruntime
if(WIN32)
    set_target_properties(onnxruntime PROPERTIES IMPORTED_LOCATION ${ONNXRUNTIME_DIR}/lib/onnxruntime.lib)
else()
    set_target_properties(onnxruntime PROPERTIES IMPORTED_LOCATION ${ONNXRUNTIME_DIR}/lib/libonnxruntime.a)
endif()

# Link onnxruntime to babylon
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
