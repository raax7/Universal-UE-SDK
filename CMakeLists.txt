cmake_minimum_required(VERSION 3.15)
project(UESDK)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)

add_subdirectory(dependencies/libhat)

set(UESDK_SRC
    "src/UESDK.cpp"
    "src/private/Memory.cpp"
    "src/private/OffsetFinder.cpp"
    "src/uesdk/FastSearch.cpp"
    "src/uesdk/FMemory.cpp"
    "src/uesdk/ObjectArray.cpp"
    "src/uesdk/UnrealObjects.cpp"
    "src/uesdk/UnrealTypes.cpp"
)

add_library(UESDK STATIC ${UESDK_SRC})

target_include_directories(UESDK PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(UESDK PUBLIC
    dependencies/libhat/include
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(UESDK PUBLIC
    libhat
)

enable_testing()
add_subdirectory(tests)
