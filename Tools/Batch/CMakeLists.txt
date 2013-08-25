# Build the minimal Spire viewer.

########################################################################
# CMake version and project name

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
INCLUDE(ExternalProject)

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

# Note: The spire tests require that ${BASE_SPIRE_DIR}/Spire is in the include path.
include_directories(${BASE_SPIRE_DIR} ${BASE_SPIRE_DIR}/Spire)

########################################################################
# Find and configure Git

find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "Git not found.")
endif()

########################################################################
# Grab Regression Data From GitHub

set(REGRESSION_DATA_DIR "${BASE_SPIRE_DIR}/Tests/RegressionData")

if(EXISTS "${REGRESSION_DATA_DIR}" AND IS_DIRECTORY "${REGRESSION_DATA_DIR}")
  message(STATUS "Updating regression data")
  execute_process(COMMAND ${GIT_EXECUTABLE} pull
    WORKING_DIRECTORY ${REGRESSION_DATA_DIR}
    RESULT_VARIABLE git_result)
else()
  message(STATUS "Downloading regression data")
  execute_process(COMMAND ${GIT_EXECUTABLE} clone git://github.com/SCIInstitute/SpireRegression ${REGRESSION_DATA_DIR}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    RESULT_VARIABLE git_result)
endif()

########################################################################
# Regression directory setup

# TODO: Make all of these options.
add_definitions(-DTEST_OUTPUT_IMAGES)
add_definitions(-DTEST_IMAGE_OUTPUT_DIR="${REGRESSION_DATA_DIR}/Output")
add_definitions(-DTEST_IMAGE_COMPARE_DIR="${REGRESSION_DATA_DIR}/Verified")
add_definitions(-DTEST_PERCEPTUAL_COMPARE)
add_definitions(-DTEST_PERCEPTUAL_COMPARE_BINARY="perceptualdiff")

########################################################################
# Compiler settings

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DSPIRE_DEBUG)
endif()

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

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
# Google Test - must come after setting compiler flags.

add_definitions(-DGTEST_USE_OWN_TR1_TUPLE=1)
add_subdirectory(${BASE_SPIRE_DIR}/Spire/3rdParty/gtest ${BASE_SPIRE_DIR}/bin/gtest)

# Include gtest
include_directories(SYSTEM ${BASE_SPIRE_DIR}/Spire/3rdParty/gtest/include)

########################################################################
# GLM

# GLM include directory.
INCLUDE_DIRECTORIES(SYSTEM "${BASE_SPIRE_DIR}/Spire/3rdParty/glm" )

########################################################################
# CImg

# GLM include directory.
INCLUDE_DIRECTORIES(SYSTEM "${BASE_SPIRE_DIR}/Tools/Batch/3rdParty/CImg" )

########################################################################
# After adding third-party libraries, turn on strict warnings.
if (APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion -Wshadow")
endif ()

########################################################################
# Setup source

# Setup spire test sources. Even though this is a batch renderer, this is
# a good test of concept.
set(CommonSources
  BatchContext.cpp
  BatchContext.h
  main.cpp)

file(GLOB TestSources
  "${BASE_SPIRE_DIR}/Tests/*.cpp"
  "${BASE_SPIRE_DIR}/Tests/*.h"
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

# Exported meshes. These are used to test rendering.
file(COPY ${BASE_SPIRE_DIR}/Spire/ExportedMeshes/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets
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

