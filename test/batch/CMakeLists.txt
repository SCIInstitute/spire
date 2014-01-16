
if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

project(SpireBatchRenderer)

# Base tests directory.
set (BASE_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../tests)

# If 'ON', This flag informs cpm-gl-batch-context that it should link against
# the OSMesa software rendering library.
option(USE_OS_MESA "Use an OS mesa context." OFF)
if (USE_OS_MESA)
  add_definitions(-DUSE_OS_MESA)
  set(MESA_LIBRARIES "OSMesa32")
endif()

#-----------------------------------------------------------------------
# Compiler settings
#-----------------------------------------------------------------------

if (CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DSPIRE_DEBUG)
endif()

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
  if (APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
  endif ()
endif ()


#------------------------------------------------------------------------------
# Required CPM Setup - See: http://github.com/iauns/cpm
#------------------------------------------------------------------------------
set(CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
find_package(Git)
if(NOT GIT_FOUND)
  message(FATAL_ERROR "CPM requires Git.")
endif()
if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
  message(STATUS "Cloning repo (https://github.com/iauns/cpm)")
  execute_process(
    COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
    RESULT_VARIABLE error_code
    OUTPUT_QUIET ERROR_QUIET)
  if(error_code)
    message(FATAL_ERROR "CPM failed to get the hash for HEAD")
  endif()
endif()
include(${CPM_DIR}/CPM.cmake)

# ++ MODULE: Our spire module
#    Be sure this module comes before batch_testing. Batch testing will
#    attempt to load the spire module if it does not already exist.
CPM_AddModule("spire"
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  SOURCE_GHOST_GIT_REPO "https://github.com/SCIInstitute/spire"
  SOURCE_GHOST_GIT_TAG "origin/master")

# ++ MODULE: OpenGL Batch Testing
CPM_AddModule("batch_testing"
  GIT_REPOSITORY "https://github.com/iauns/cpm-gl-batch-testing"
  GIT_TAG "1.1.0")

# ++ EXTERNAL-MODULE: Google Test
CPM_AddModule("google_test"
  GIT_REPOSITORY "https://github.com/iauns/cpm-google-test"
  GIT_TAG "1.0.0"
  USE_EXISTING_VER TRUE)

CPM_Finish()

#-----------------------------------------------------------------------
# Grab Regression Data From GitHub
#-----------------------------------------------------------------------

set(REGRESSION_DATA_DIR "${BASE_TESTS_DIR}/RegressionData")

CPM_EnsureRepoIsCurrent(
  TARGET_DIR ${REGRESSION_DATA_DIR}
  GIT_REPOSITORY "https://github.com/SCIInstitute/SpireRegression"
  GIT_TAG "origin/master"
  )

#-----------------------------------------------------------------------
# Regression directory setup
#-----------------------------------------------------------------------

# TODO: Make all of these options.
add_definitions(-DTEST_OUTPUT_IMAGES)
add_definitions(-DTEST_IMAGE_OUTPUT_DIR="${REGRESSION_DATA_DIR}/Output")
add_definitions(-DTEST_IMAGE_COMPARE_DIR="${REGRESSION_DATA_DIR}/Verified")
add_definitions(-DTEST_PERCEPTUAL_COMPARE)
add_definitions(-DTEST_PERCEPTUAL_COMPARE_BINARY="perceptualdiff")

#-----------------------------------------------------------------------
# Setup strict warnings and werror
#-----------------------------------------------------------------------

if(APPLE)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wconversion -Wshadow")
endif()

#-----------------------------------------------------------------------
# Setup source
#-----------------------------------------------------------------------

# Setup spire test sources. Even though this is a batch renderer, this is
# a good test of concept.
set(CommonSources
  main.cpp)

file(GLOB TestSources
  "${BASE_TESTS_DIR}/*.cpp"
  "${BASE_TESTS_DIR}/*.h"
  )

########################################################################
# Copy shaders to the target directory

# Copy shaders.
file(COPY ${BASE_TESTS_DIR}/shaders/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

# Exported meshes. These are used to test rendering.
file(COPY ${BASE_TESTS_DIR}/assets/
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
  ${SpecificSources}
  ${Source_GLEW_for_Windows})

target_link_libraries(spirebatch 
  gtest
  ${CPM_LIBRARIES}
  ${MESA_LIBRARIES}
  ${OPENGL_LIBRARIES}
  ${OS_SPECIFIC_LIBS})

