if(APPLE)
  cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
else()
  cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
endif()

project (AssetConverter)

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

# ++ MODULE: GLM
CPM_AddModule("glm"
  GIT_REPOSITORY "https://github.com/iauns/cpm-glm"
  GIT_TAG "1.0.2")

CPM_Finish()

#------------------------------------------------------------------------------
# TCLAP
#------------------------------------------------------------------------------
set (TCLAP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../3rdParty/tclap/include")

# Libraries / Includes / Definitions
include_directories("${TCLAP_INCLUDE_DIR}")


#------------------------------------------------------------------------------
# BOOST
#------------------------------------------------------------------------------

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
FIND_PACKAGE( Boost 1.50.0 COMPONENTS filesystem system REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

#------------------------------------------------------------------------------
# Common Variables
#------------------------------------------------------------------------------

set (BASE_SPIRE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

#------------------------------------------------------------------------------
# Compiler settings
#------------------------------------------------------------------------------

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif ()

#------------------------------------------------------------------------------
# Source
#------------------------------------------------------------------------------

# All source files in the current directory.
file(GLOB Source_ProjectSourceDir
  "*.h"
  "*.cpp"
  )
set(EXE_NAME ${PROJECT_NAME}_r)

add_executable( ${EXE_NAME} ${Source_ProjectSourceDir})

if (APPLE)
  target_link_libraries(${EXE_NAME} assimp IL ILU jpeg tiff jasper ${Boost_LIBRARIES})
endif()

