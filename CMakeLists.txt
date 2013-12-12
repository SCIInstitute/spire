#-----------------------------------------------------------------------
# CMake configuration
#-----------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)

#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
set(CPM_MODULE_NAME "spire")
set(CPM_LIB_TARGET_NAME ${CPM_MODULE_NAME})

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_TARGET_NAME))
  set(CPM_LIB_TARGET_NAME ${CPM_TARGET_NAME})
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
  include(CPM)
else()
  # Clone CPM.
  set (CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
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
endif()

# TODO: Auto-download GLEW on windows only since the other platforms don't need it.
#       (https://github.com/nigels-com/glew.git)

# ++ MODULE: GLM
CPM_AddModule("GLM"
  GIT_REPOSITORY "https://github.com/iauns/cpm-glm"
  GIT_TAG "1.0.1"
  USE_EXISTING_VER TRUE
  EXPORT_MODULE TRUE    # Use EXPORT_MODULE sparingly. We expose GLM's interface
  )                     # through our own interface hence why we export it.

CPM_AddModule("gl_platform"
  GIT_REPOSITORY "https://github.com/iauns/cpm-gl-platform.git"
  GIT_TAG "1.2.0"
  EXPORT_MODULE TRUE
  USE_EXISTING_VER TRUE)

## ++ MODULE: C++11 implementation of optional
#CPM_AddModule("optional"
#  GIT_REPOSITORY "https://github.com/iauns/cpm-optional"
#  GIT_TAG "origin/master"
#  USE_EXISTING_VER TRUE
#  EXPORT_MODULE TRUE
#  )

# Additional include directories the parent needs to know about. But ONLY
# on windows where we require GLEW to function properly.
CPM_ExportAdditionalIncludeDir("${CMAKE_CURRENT_SOURCE_DIR}/spire/3rdParty")

# This call will ensure all include directories and definitions are present
# in the target. These correspond to the modules that we added above.
CPM_InitModule(${CPM_MODULE_NAME})

#-----------------------------------------------------------------------
# CMake Build Options
#-----------------------------------------------------------------------

option(USE_GOOGLE_PERF "Use google perf tools to profile Spire." OFF)
option(USE_CORE_PROFILE_3 "Use OpenGL core profile 3.2 (supported by most macs)." OFF)
option(USE_CORE_PROFILE_4 "Use OpenGL core profile 4.1 (supported by mac 10.9)." OFF)

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DSPIRE_DEBUG)
endif()

if (USE_GOOGLE_PERF)
  add_definitions(-DSPIRE_PROFILE)
  set(PROFILE_LIBS profiler)
endif()

if(USE_CORE_PROFILE_3)
  add_definitions(-DUSE_CORE_PROFILE_3)
endif()

if(USE_CORE_PROFILE_4)
  add_definitions(-DUSE_CORE_PROFILE_4)
endif()

#-----------------------------------------------------------------------
# OpenGL configuration
#-----------------------------------------------------------------------

if(NOT DEFINED IOS)
  find_package(OpenGL REQUIRED)
endif()

#-----------------------------------------------------------------------
# Compiler settings
#-----------------------------------------------------------------------
if(UNIX)
  if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    if(BUILDING_IOS)
      # No Werror on iOS builds yet.
      set (CMAKE_CXX_FLAGS 
        "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Wconversion -Wshadow -Wunreachable-code")
    else()
      set (CMAKE_CXX_FLAGS 
        "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Werror -Wconversion -Wshadow -Wunreachable-code")
    endif()

  else(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
    add_definitions(-D_GLIBCXX_USE_NANOSLEEP)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif(APPLE)

else(UNIX)
  if(WIN32)
    # Get rid of a bunch of windows warnings.

    # Disables needs to have dll-interface to be used by clients warning.
    # This was happening on STL classes and leading to a lot of noise. While
    # this is a valid warning if you want your DLL to be reusable across applications
    # this is NOT what we want for Spire. Spire should be built side-by-ide with
    # your application, using the same compiler and standard library.
    add_definitions("/wd4251")

    # Getting rid of unknown pragram warning. This warning isn't issued
    # on any other compiler and is generally a source of noise if you are
    # doing cross platform work.
    add_definitions("/wd4068")

    # Need this definition for Visual Studio to compile std::bind with more
    # than 4 arguments.
    add_definitions("-D_VARIADIC_MAX=10")

    # GLEW
    # GLEW is required on windows since Microsoft has not updated the OpenGL
    # headers since OpenGL 1.1.

    # Since we are building a static library, we need to define GLEW_STATIC
    # <url:http://glew.sourceforge.net/install.html#^define the GLEW_STATIC>
    add_definitions(-DGLEW_STATIC)

    # GLEW include directories and source.
    include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glew/include)
    set(Source_GLEW_for_Windows 3rdParty/glew/src/glew.c)

  endif(WIN32)
endif(UNIX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Indicate that headers from 3rdParty libraries should be treated as system files.
# This suppresses warnings in clang. With -Werror, this is mandatory for repos
# which I do not manage.
# See: http://clang.llvm.org/docs/UsersManual.html#controlling-errors-and-warnings
include_directories(SYSTEM
  ${CMAKE_CURRENT_SOURCE_DIR}/spire/3rdParty
  ${OPENGL_INCLUDE_DIRS})

#-----------------------------------------------------------------------
# Source
#-----------------------------------------------------------------------

# Globbing has some downsides, but the advantages outweigh the
# disadvantages.
file (GLOB Source_Root
  "spire/*.cpp"
  "spire/*.h"
  )

file (GLOB Source_Core
  "spire/src/*.cpp"
  "spire/src/*.h"
  )

#-----------------------------------------------------------------------
# Library setup
#-----------------------------------------------------------------------

# Build the library.
add_library(${CPM_LIB_TARGET_NAME}
  ${Source_Root}
  ${Source_Core}
  ${Source_GLEW_for_Windows}
  )

# Make sure we don't link against OpenGL libraries if using OSMesa
if (USE_OS_MESA)
  set(OPENGL_LIBRARIES)
endif()

# These libraries likely have no effect in the final build.
target_link_libraries(
  ${CPM_LIB_TARGET_NAME}
  ${OPENGL_LIBRARIES}
  ${CPM_LIBRARIES}
  )

