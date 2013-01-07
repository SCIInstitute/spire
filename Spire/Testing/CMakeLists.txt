# Testing application for spire.

########################################################################
# CMake version and project name

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(SpireTester)

########################################################################
# Configure OpenGL

find_package(OpenGL REQUIRED)

########################################################################
# Common Variables

set (BASE_SPIRE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

########################################################################
# Required libraries

# The 'spire' library.
add_subdirectory(${BASE_SPIRE_DIR}/Spire ${BASE_SPIRE_DIR}/bin/spire)

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

  if (APPLE)
    add_definitions(-DUSING_OSX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  else()
    add_definitions(-DUSING_LINUX)
  endif ()

endif ()

add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1 -DSPIRE_IS_TESTING)
include_directories(${BASE_SPIRE_DIR}/Spire/3rdParty/gtest/include ${BASE_SPIRE_DIR}/Spire)

########################################################################
# Google test must come after setting compiler flags.

add_subdirectory(${BASE_SPIRE_DIR}/Spire/3rdParty/gtest ${BASE_SPIRE_DIR}/bin/gtest)

########################################################################
# Setup source

# Be aware of the downsides of file globbing before using.
file (GLOB Source_Root
  "*.cpp"
  "*.h"
  )

file (GLOB Source_High
  "../High/Tests/*.cc"
  "../High/Tests/*.h"
  )

file (GLOB Source_Low
  "../Low/Tests/*.cc"
  "../Low/Tests/*.h"
  )

file (GLOB Source_Caching
  "../Caching/Tests/*.cc"
  "../Caching/Tests/*.h"
  )

file (GLOB Source_Pipes
  "../Pipes/Tests/*.cc"
  "../Pipes/Tests/*.h"
  "../Pipes/StuPipe/Tests/*.cc"
  "../Pipes/StuPipe/Tests/*.h"
  )

########################################################################
# Setup executable

add_executable(SpireTest 
  ${Source_Root}
  ${Source_High}
  ${Source_Low}
  ${Source_Caching}
  ${Source_Pipes})

target_link_libraries(SpireTest 
  Spire 
  gtest
  ${OPENGL_LIBRARIES})

