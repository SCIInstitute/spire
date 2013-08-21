# Builds the spire library and all of its dependencies.

########################################################################
# CMake configuration

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
include(ExternalProject)

########################################################################
# CMake Build Options

option(USE_SCIRUN_INTERFACE "Includes custom interface to SCIRun." OFF)
option(USE_TUVOK "Builds and includes the Tuvok volume renderer" OFF)
if(WIN32)
  option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." OFF)
else()
  option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." ON)
endif()
option(USE_GOOGLE_PERF "Use google perf tools to profile Spire." OFF)
option(USE_CORE_PROFILE_3 "Use OpenGL core profile 3.2 (supported by most macs)." OFF)
option(USE_CORE_PROFILE_4 "Use OpenGL core profile 4.1 (supported by mac 10.9)." OFF)

if(USE_TUVOK)
  add_definitions(-DSPIRE_USE_TUVOK)
endif()

if(USE_STD_THREADS)
  add_definitions(-DSPIRE_USE_STD_THREADS)
endif()

if(USE_SCIRUN_INTERFACE)
  add_definitions(-DSPIRE_USE_SCIRUN_INTERFACE)
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

########################################################################
# OpenGL configuration

if(NOT DEFINED IOS)
  find_package(OpenGL REQUIRED)
endif()

########################################################################
# Compiler settings
if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

  if(APPLE)
    if(BUILDING_IOS)
      # No Werror on iOS builds yet.
      set (CMAKE_CXX_FLAGS 
        "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Wconversion -Wshadow -Wunreachable-code")
    else()
      set (CMAKE_CXX_FLAGS 
        "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Werror -Wconversion -Wshadow -Wunreachable-code")
    endif()

    # We were using CoreFoundation to look up shaders in the bundle.
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    set(OS_SPECIFIC_LIBS ${CORE_FOUNDATION_FRAMEWORK})

    if(IOS)
      add_definitions(-DSPIRE_USING_IOS)
      add_definitions(-DSPIRE_OPENGL_ES_2)
    else()
      add_definitions(-DSPIRE_USING_OSX)
    endif()
  else(APPLE)
    add_definitions(-DSPIRE_USING_LINUX -D_GLIBCXX_USE_NANOSLEEP)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  endif(APPLE)

  # Super strict settings.
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
else(UNIX)
  if(WIN32)
    add_definitions(-DSPIRE_USING_WIN)
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

########################################################################
# Eigen

# Eigen include directory.
include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/eigen" )
add_definitions(-DEIGEN_INITIALIZE_MATRICES_BY_ZERO)

########################################################################
# GLM

# GLM include directory.
INCLUDE_DIRECTORIES(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glm" )

########################################################################
# Tuvok

if(USE_TUVOK)
  # FIND QT -- Required for compiling tuvok...
  # Tuvok's build system needs to be updated in the future so that it can
  # use builders other than QT. There is no reason to have a QT dependency
  # just for generating a makefile for Tuvok.
  set(QT_MIN_VERSION "4.8.0")
  find_package(Qt4 REQUIRED)

  if(QT_FOUND)
  else()
    message(FATAL_ERROR "QT 4.9 or greater is required to build Spire (for Tuvok)")
  endif()

  # Git (for checking out external projects).
  find_package(Git)
  if(NOT GIT_FOUND)
    MESSAGE(FATAL_ERROR "Git not found.")
  endif()

  # Svn
  include(FindSubversion)

  # Tuvok
  # See: http://cmake.org/cmake/help/v2.8.8/cmake.html#module:ExternalProject
  set(TuvokSourceDir "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Tuvok")
  set(TuvokBinaryDir "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/Tuvok")

  if(UNIX)

    if(APPLE)
      # TODO: Get rid of unsupported macx/clang when we switch to qt5.
      # Note: There are no quotes around the command. If there were,
      #       sh would interpret it as a file
      set(TuvokConfigureStep ${QT_QMAKE_EXECUTABLE} -spec unsupported/macx-clang QMAKE_CONFIG+='release' QMAKE_CFLAGS='-O2' QMAKE_CXXFLAGS='-O2' QMAKE_LFLAGS='' -recursive ${TuvokSourceDir}/Tuvok.pro)
      set(TuvokMakeStep make -j2)
    else(APPLE)

    endif(APPLE)

    # Ensure we can link against libTuvok.a
    set(TUVOK_LIB ${TuvokBinaryDir}/Build/libTuvok.a)

    # Note: The build/configure command will vary based on platform!
    # Specify appropriate Tuvok SVN revision when we get a stable version.
    # We set UPDATE_COMMAND to "" because we don't want to svn update everytime
    # we build the project.
    ExternalProject_Add(ThirdParty_Tuvok
      PREFIX ${TuvokBinaryDir}
      SVN_REPOSITORY "https://gforge.sci.utah.edu/svn/Tuvok"
      SVN_TRUST_CERT 1
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/Tuvok"
      UPDATE_COMMAND ""
      SOURCE_DIR ${TuvokSourceDir}
      BINARY_DIR ${TuvokBinaryDir}
      CONFIGURE_COMMAND ${TuvokConfigureStep}
      BUILD_COMMAND ${TuvokMakeStep}
      )

  else(UNIX)
    if(WIN32)

      # TODO Figure out how to build Tuvok with the sln files on windows OR
      #      fix tuvok so that it uses qmake to generate the sln files, just
      #      like unix.

    else(WIN32)
      MESSAGE(FATAL_ERROR "Unknown platform.")
    endif(WIN32)
  endif(UNIX)
endif(USE_TUVOK)

########################################################################
# Source

# Globbing has some downsides, but the advantages outweigh the
# disadvantages.
file (GLOB Source_Root
  "*.cpp"
  "*.h"
  )

file (GLOB Source_High
  "Core/*.cpp"
  "Core/*.h"
  )

file (GLOB Source_Low
  "Low/*.cpp"
  "Low/*.h"
  )

file (GLOB Source_Caching
  "Caching/*.cpp"
  "Caching/*.h"
  )

if(USE_SCIRUN_INTERFACE)
  file (GLOB Source_SCIRun
    "AppSpecific/SCIRun/*.cpp"
    "AppSpecific/SCIRun/*.h"
    )
endif()

########################################################################
# Library setup

# Build the library or the testing app.
set(SPIRE_TARGET_NAME Spire)
add_library(${SPIRE_TARGET_NAME}
  ${Source_Root}
  ${Source_High}
  ${Source_Low}
  ${Source_Caching}
  ${Source_Testing}
  ${Source_SCIRun}
  ${Source_GLEW_for_Windows}
  )
add_dependencies(${SPIRE_TARGET_NAME} ThirdParty_Tuvok)

target_link_libraries(
  ${SPIRE_TARGET_NAME}
  ${OPENGL_LIBRARIES}
  ${TUVOK_LIB}
  ${OS_SPECIFIC_LIBS}
  ${TESTING_LIBS}
  ${PROFILING_LIBS}
  )

