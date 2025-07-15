function(_add_compile_test testname sourcefile will_fail)
  add_executable(${testname} "${sourcefile}")

  target_include_directories(${testname} PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
  )

  set_target_properties(${testname} PROPERTIES 
    EXCLUDE_FROM_ALL TRUE
    EXCLUDE_FROM_DEFAULT_BUILD TRUE
  )

  target_link_libraries(${testname} PRIVATE UESDK)

  add_test(NAME ${testname}
    COMMAND ${CMAKE_COMMAND} --build "${CMAKE_BINARY_DIR}" --target ${testname} --config $<CONFIGURATION>)

  if(will_fail)
    set_tests_properties(${testname} PROPERTIES WILL_FAIL TRUE)
  endif()
endfunction()

function(compile_fail testname sourcefile)
  _add_compile_test(${testname} ${sourcefile} TRUE)
endfunction()

function(compile_pass testname sourcefile)
  _add_compile_test(${testname} ${sourcefile} FALSE)
endfunction()

# static_assert pass tests
compile_pass(pass_simpleargs "pass_simpleargs.cpp")

# static_assert fail tests
compile_fail(fail_uncopyablearg "fail_uncopyablearg.cpp")
