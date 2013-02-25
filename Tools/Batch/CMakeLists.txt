# Build the minimal Spire viewer.

########################################################################
# CMake version and project name

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(SpireBatchRenderer)

########################################################################
# Configure OpenGL
find_package(OpenGL REQUIRED)

########################################################################
# Common Variables

set (BASE_SPIRE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

########################################################################
# Required libraries

# The 'spire' library.
add_subdirectory(${BASE_SPIRE_DIR}/Spire ${CMAKE_CURRENT_BINARY_DIR}/spire)

# Pull USE_STD_THREADS from the spire directory
get_directory_property(USE_STD_THREADS DIRECTORY ${BASE_SPIRE_DIR}/Spire DEFINITION USE_STD_THREADS)
if(USE_STD_THREADS)
  add_definitions(-DSPIRE_USE_STD_THREADS)
endif()

get_directory_property(USI_SCIRUN_INTERFACE DIRECTORY ${BASE_SPIRE_DIR}/Spire DEFINITION USE_SCIRUN_INTERFACE)
if (USE_SCIRUN_INTERFACE)
  add_definitions(-DSPIRE_USE_SCIRUN_INTERFACE)
endif()

# Include gtest
add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)
include_directories(${BASE_SPIRE_DIR}/Spire/3rdParty/gtest/include ${BASE_SPIRE_DIR} ${BASE_SPIRE_DIR}/Spire)
# Note: The spire tests require that ${BASE_SPIRE_DIR}/Spire is in the include path.

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

  if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
    add_definitions(-DSPIRE_USING_OSX)

    # Find necessary apple libraries (cocoa).
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    set(OS_SPECIFIC_LIBS ${COCOA_LIBRARY})

    # Apple specific sources
    set (SpecificSources
      CGLContext.cpp
      CGLContext.h
      NSContext.mm
      NSContext.h
      )
  else ()
    # Catch all is linux for other unix platforms...
    add_definitions(-DSPIRE_USING_LINUX)

    # Linux specific sources
    set (SpecificSources
      GLXContext.cpp
      GLXContext.h
      )
  endif ()
else()
  if (WIN32)
    add_definitions(-DSPIRE_USING_WIM)

    # Windows specific sources
    set (SpecificSources
      WGLContext.cpp
      WGLContext.h
      )
  endif(WIN32)
endif ()

########################################################################
# Google test must come after setting compiler flags.

add_subdirectory(${BASE_SPIRE_DIR}/Spire/3rdParty/gtest ${BASE_SPIRE_DIR}/bin/gtest)

########################################################################
# Setup source

# Setup spire test sources. Even though this is a batch renderer, this is
# a good test of concept.
set(CommonSources
  BatchContext.cpp
  BatchContext.h
  main.cpp)

file(GLOB TestSources
  "${BASE_SPIRE_DIR}/Spire/Tests/*.cc"
  "${BASE_SPIRE_DIR}/Spire/Tests/*.h"
  )

########################################################################
# Copy shaders to the target directory

# Copy shaders.
file(COPY ${BASE_SPIRE_DIR}/Spire/Shaders/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

########################################################################
# Setup executable

add_executable(spirebatch
  ${CommonSources}
  ${TestSources}
  ${SpecificSources})

add_dependencies(spirebatch Spire)

target_link_libraries(spirebatch 
  Spire
  gtest
  ${OPENGL_LIBRARIES}
  ${OS_SPECIFIC_LIBS})

