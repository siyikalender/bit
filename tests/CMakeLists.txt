# Tests of the bit library
#
# This is free and unencumbered software released into the public domain.
# For more information, please refer to <http://unlicense.org> and the
# LICENSE file in the root of this repository.
#
# Author: Claude Opus 5.5 <noreply@anthropic.com>
# Date:   2026

# Run time tests. Most checks are static_asserts, so building is already
# part of the test.
foreach(test test_field test_storage test_atomic_storage test_register)
  add_executable(bit_${test} ${test}.cpp)
  target_link_libraries(bit_${test} PRIVATE bit::bit bit_warnings Threads::Threads)
  add_test(NAME ${test} COMMAND bit_${test})
endforeach()

# Every public header must compile on its own and survive being included twice
set(BIT_HEADERS field pack storage atomic_storage register)
set(BIT_HEADER_CHECK_SOURCES "")

foreach(header ${BIT_HEADERS})
  set(source ${CMAKE_CURRENT_BINARY_DIR}/header_check/${header}.cpp)
  file(WRITE ${source}.in
    "#include \"bit/${header}.hpp\"\n#include \"bit/${header}.hpp\"\n")
  configure_file(${source}.in ${source} COPYONLY)
  list(APPEND BIT_HEADER_CHECK_SOURCES ${source})
endforeach()

add_library(bit_header_check OBJECT ${BIT_HEADER_CHECK_SOURCES})
target_link_libraries(bit_header_check PRIVATE bit::bit bit_warnings)

# In debug builds an oversized value makes assign assert.
# Skipped on MSVC, whose debug runtime may open a dialog on abort.
if(NOT MSVC)
  add_executable(bit_assert_overflow assert_overflow.cpp)
  target_link_libraries(bit_assert_overflow PRIVATE bit::bit bit_warnings)
  add_test(NAME assert_overflow COMMAND bit_assert_overflow)
  set_tests_properties(assert_overflow PROPERTIES
    PASS_REGULAR_EXPRESSION "value does not fit in field"
    SKIP_REGULAR_EXPRESSION "BIT_TEST_SKIPPED_NDEBUG")
endif()

# Compile failure tests: each source must fail to build with the expected
# diagnostic. The target is excluded from the normal build and built by the
# test itself; matching the message makes sure it fails for the right reason.
function(bit_compile_fail name expected)
  add_executable(bit_cf_${name} EXCLUDE_FROM_ALL compile_fail/${name}.cpp)
  target_link_libraries(bit_cf_${name} PRIVATE bit::bit)

  add_test(NAME compile_fail.${name}
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
                             --target bit_cf_${name}
                             --config $<CONFIG>)

  set_tests_properties(compile_fail.${name} PROPERTIES
    PASS_REGULAR_EXPRESSION "${expected}"
    SKIP_REGULAR_EXPRESSION "BIT_TEST_SKIPPED_NDEBUG"
    # Builds in the same build tree must not run in parallel
    RUN_SERIAL TRUE)
endfunction()

bit_compile_fail(field_order             "First index cannot be greater than Last index")
bit_compile_fail(fields_overlap          "Fields overlap")
bit_compile_fail(field_not_in_pack       "Field not in pack")
bit_compile_fail(field_exceeds_width     "Field exceeds value type width")
bit_compile_fail(signed_value_type       "Pack value type must be uint8_t")
bit_compile_fail(atomic_value_type       "for thread safety use bit::atomic_storage")
bit_compile_fail(constant_does_not_fit   "Value does not fit in field")
bit_compile_fail(constexpr_assign_overflow
  "constant expression|non-'constexpr'|did not evaluate to a constant")
bit_compile_fail(compare_different_packs
  "operator==|invalid operands|binary '=='")
bit_compile_fail(atomic_storage_copy     "deleted")
