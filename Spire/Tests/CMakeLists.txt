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

# Google Test
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/gtest)

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

########################################################################
# Setup source

# Be aware of the downsides of file globbing before using.
file (GLOB Source_Root
  "*.cpp"
  "*.h"
  )

file (GLOB Source_High
  "../High/Tests/*.cpp"
  "../High/Tests/*.h"
  )

file (GLOB Source_Low
  "../Low/Tests/*.cpp"
  "../Low/Tests/*.h"
  )

file (GLOB Source_Caching
  "../Caching/Tests/*.cpp"
  "../Caching/Tests/*.h"
  )

file (GLOB Source_Pipes
  "../Pipes/Tests/*.cpp"
  "../Pipes/Tests/*.h"
  "../Pipes/StuPipe/Tests/*.cpp"
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
  ${OPENGL_LIBRARIES})

