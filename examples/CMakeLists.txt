project(UESDK-Examples)

file(GLOB_RECURSE EXAMPLE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
file(GLOB_RECURSE EXAMPLE_HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

set(EXAMPLE_VARIANTS
    UE4
    UE5
)

function(add_example_variant VARIANT)
    set(TARGET_NAME "UESDK-Examples-${VARIANT}")

    add_library(${TARGET_NAME} SHARED ${EXAMPLE_SOURCES} ${EXAMPLE_HEADERS})

    target_include_directories(${TARGET_NAME} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE ${VARIANT})

    target_link_libraries(${TARGET_NAME} PRIVATE
        uesdk
    )
endfunction()

foreach(V ${EXAMPLE_VARIANTS})
    add_example_variant(${V})
endforeach()
