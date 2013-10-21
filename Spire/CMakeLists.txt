#-----------------------------------------------------------------------
# CMake configuration
#-----------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.7 FATAL_ERROR)
include(ExternalProject)

#-----------------------------------------------------------------------
# CPM configuration
#-----------------------------------------------------------------------
set(LIB_TARGET_NAME Spire)

# To build this project you don't need to use CPM. This section is only
# for those who wish to use CPM to configure and build Spire.

if ((DEFINED CPM_DIR) AND (DEFINED CPM_UNIQUE_ID) AND (DEFINED CPM_OUTPUT_LIB_NAME))
  # We are being built as a sub project. Use the primary project's CPM
  # so that we can take advantage of possible duplicate module versions.
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CPM_DIR})
  include(CPM)

  # Set the target based on the output library name.
  set(LIB_TARGET_NAME ${CPM_OUTPUT_LIB_NAME})
  add_definitions("-DCPM_NAMESPACE_NAME=${CPM_UNIQUE_ID}")
else()
  # Clone CPM.
  set (CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)

  find_package(Git)
  if(NOT GIT_FOUND)
    message(FATAL_ERROR "CPM requires Git.")
  endif()
  if (NOT EXISTS ${CPM_DIR}/CPM.cmake)
    execute_process(
      COMMAND "${GIT_EXECUTABLE}" clone https://github.com/iauns/cpm ${CPM_DIR}
      RESULT_VARIABLE error_code
      OUTPUT_VARIABLE head_sha
      )
    if(error_code)
      message(FATAL_ERROR "CPM failed to get the hash for HEAD")
    endif()
  endif()
  include(${CPM_DIR}/CPM.cmake)
endif()

# Include CPM modules here.

# Do the following only if we include spire...
# do this AFTER we have included all other modules.
include_directories(${CPM_INCLUDE_DIRS})
CPM_INIT_MODULE("spire")

# Normally we would add CPM modules here. But we aren't using CPM to package
# any external modules. We are just a bare CPM module with 0 CPM dependencies.

#-----------------------------------------------------------------------
# CMake Build Options
#-----------------------------------------------------------------------

if(WIN32)
  option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." OFF)
else()
  option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." ON)
endif()
option(USE_GOOGLE_PERF "Use google perf tools to profile Spire." OFF)
option(USE_CORE_PROFILE_3 "Use OpenGL core profile 3.2 (supported by most macs)." OFF)
option(USE_CORE_PROFILE_4 "Use OpenGL core profile 4.1 (supported by mac 10.9)." OFF)

if(USE_STD_THREADS)
  add_definitions(-DSPIRE_USE_STD_THREADS)
endif()

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
  ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty
  ${OPENGL_INCLUDE_DIRS})

#-----------------------------------------------------------------------
# Eigen
#-----------------------------------------------------------------------

# Eigen include directory.
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/eigen" )
add_definitions(-DEIGEN_INITIALIZE_MATRICES_BY_ZERO)

#-----------------------------------------------------------------------
# GLM
#-----------------------------------------------------------------------

# GLM include directory.
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glm" )

#-----------------------------------------------------------------------
# Source
#-----------------------------------------------------------------------

# Globbing has some downsides, but the advantages outweigh the
# disadvantages.
file (GLOB Source_Root
  "*.cpp"
  "*.h"
  )

file (GLOB Source_Core
  "Core/*.cpp"
  "Core/*.h"
  )

#-----------------------------------------------------------------------
# Library setup
#-----------------------------------------------------------------------

# Build the library.
add_library(${LIB_TARGET_NAME}
  ${Source_Root}
  ${Source_Core}
  ${Source_GLEW_for_Windows}
  )

# These libraries likely have no effect in the final build.
target_link_libraries(
  ${LIB_TARGET_NAME}
  ${OPENGL_LIBRARIES}
  ${CPM_LIBRARIES}
  )

