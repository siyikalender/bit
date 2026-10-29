# Builds the examples and runs each of them as a smoke test
#
# This is free and unencumbered software released into the public domain.
# For more information, please refer to <http://unlicense.org> and the
# LICENSE file in the root of this repository.
#
# Author: Claude Opus 5.5 <noreply@anthropic.com>
# Date:   2026

foreach(example example_1 example_2 example_3 example_4 example_5)
  add_executable(bit_${example} ${example}.cpp)
  target_link_libraries(bit_${example} PRIVATE bit::bit bit_warnings Threads::Threads)

  if(BIT_BUILD_TESTS)
    add_test(NAME example.${example} COMMAND bit_${example})
  endif()
endforeach()
