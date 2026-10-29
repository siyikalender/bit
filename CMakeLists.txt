# CMake build of the bit library
#
# This is free and unencumbered software released into the public domain.
# For more information, please refer to <http://unlicense.org> and the
# LICENSE file in the root of this repository.
#
# Author: Claude Opus 5.5 <noreply@anthropic.com>
# Date:   2026

cmake_minimum_required(VERSION 3.14)

project(bit LANGUAGES CXX)

include(GNUInstallDirs)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  set(BIT_IS_TOP_LEVEL ON)
else()
  set(BIT_IS_TOP_LEVEL OFF)
endif()

option(BIT_BUILD_TESTS            "Build the bit tests"                  ${BIT_IS_TOP_LEVEL})
option(BIT_BUILD_EXAMPLES         "Build the bit examples"               ${BIT_IS_TOP_LEVEL})
option(BIT_WARNINGS_AS_ERRORS     "Treat warnings in tests as errors"    OFF)
option(BIT_INSTALL                "Generate the install target"          ${BIT_IS_TOP_LEVEL})

# Header only library: bit::bit
add_library(bit INTERFACE)
add_library(bit::bit ALIAS bit)

target_include_directories(bit
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_features(bit INTERFACE cxx_std_17)

if(BIT_INSTALL)
  install(DIRECTORY include/bit
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

  install(TARGETS bit
          EXPORT bitTargets)

  # bit has no dependencies, so the exported targets are the whole package
  # configuration: find_package(bit) then target_link_libraries(... bit::bit)
  install(EXPORT bitTargets
          FILE bitConfig.cmake
          NAMESPACE bit::
          DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/bit)
endif()

if(BIT_BUILD_TESTS OR BIT_BUILD_EXAMPLES)
  # Warning flags for the code in this repository (tests and examples)
  add_library(bit_warnings INTERFACE)

  if(MSVC)
    target_compile_options(bit_warnings INTERFACE /W4 /permissive-)
    if(BIT_WARNINGS_AS_ERRORS)
      target_compile_options(bit_warnings INTERFACE /WX)
    endif()
  else()
    target_compile_options(bit_warnings INTERFACE
      -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
    if(BIT_WARNINGS_AS_ERRORS)
      target_compile_options(bit_warnings INTERFACE -Werror)
    endif()
  endif()

  find_package(Threads REQUIRED)
endif()

if(BIT_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(BIT_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
