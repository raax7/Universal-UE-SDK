cmake_minimum_required(VERSION 3.15)
project(uesdk)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

set(BUILD_EXAMPLES OFF CACHE BOOL "Build examples")

add_subdirectory(dependencies/libhat)

set(UESDK_SRC
    "src/uesdk.cpp"
    "src/private/Memory.cpp"
    "src/private/OffsetFinder.cpp"
    "src/uesdk/core/FMemory.cpp"
    "src/uesdk/core/ObjectArray.cpp"
    "src/uesdk/core/UnrealObjects.cpp"
    "src/uesdk/core/UnrealTypes.cpp"
    "src/uesdk/helpers/FastSearch.cpp"
    "src/uesdk/helpers/TlsArgBuffer.cpp"
)

add_library(uesdk STATIC ${UESDK_SRC})

target_include_directories(uesdk PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(uesdk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}dependencies/libhat/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(uesdk PUBLIC
    libhat
)

if (BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
