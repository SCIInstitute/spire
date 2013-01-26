# Builds the spire library and all of its dependencies.

########################################################################
# CMake configuration

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
include(ExternalProject)

########################################################################
# CMake Build Options

option(USE_TUVOK "Builds and includes the Tuvok volume renderer" ON)
option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." ON)

if(USE_TUVOK)
  add_definitions(-DSPIRE_USE_TUVOK)
endif()

if(USE_STD_THREADS)
  add_definitions(-DSPIRE_USE_STD_THREADS)
endif()

if(CMAKE_BUILD_TYPE MATCHES DEBUG)
  add_definitions(-DSPIRE_DEBUG)
endif()

########################################################################
# OpenGL configuration

if(NOT DEFINED IOS)
  find_package(OpenGL REQUIRED)
endif()

########################################################################
# Compiler settings

if(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

  if(APPLE)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

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
    add_definitions(-DSPIRE_USING_LINUX)
  endif(APPLE)

  # Super strict settings.
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
else(UNIX)
  if(WIN32)
    add_definitions(-DSPIRE_USING_WIN)
    # GLEW Windows only: Since we are building a static library, we need
    # to define GLEW_STATIC <url:http://glew.sourceforge.net/install.html#^define the GLEW_STATIC>
    add_definitions(-DGLEW_STATIC)
  endif(WIN32)
endif(UNIX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/glew/include ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty ${OPENGL_INCLUDE_DIRS})

########################################################################
# Tuvok

if(USE_TUVOK)
  # FIND QT -- Required for compiling tuvok...
  # Tuvok's build system needs to be updated in the future so that it can
  # use builders other than QT. There is no reason to have a QT dependency
  # just for generating a makefile for Tuvok.
  set(QT_MIN_VERSION "4.7.0")
  find_package(Qt4 REQUIRED)

  if(QT_FOUND)
  else()
    message(FATAL_ERROR "QT 4.7 or greater is required to build Spire (for Tuvok)")
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
  "High/*.cpp"
  "High/*.h"
  )

file (GLOB Source_Low
  "Low/*.cpp"
  "Low/*.h"
  )

file (GLOB Source_Caching
  "Caching/*.cpp"
  "Caching/*.h"
  )

file (GLOB Source_Pipes
  "Shaders/Tests/*.cpp"
  "StuPipe/*.cpp"
  "StuPipe/*.h"
  )

# GLEW is not support on mobile platforms.
if(NOT DEFINED IOS)
  set(Source_GLEW
    3rdParty/glew/src/glew.c)
endif()

########################################################################
# Library setup

add_library(Spire
  ${Source_Root}
  ${Source_High}
  ${Source_Low}
  ${Source_Caching}
  ${Source_Pipes}
  ${Source_GLEW}
  )
add_dependencies(Spire ThirdParty_Tuvok)
target_link_libraries(
  Spire
  ${OPENGL_LIBRARIES}
  ${TUVOK_LIB}
  ${OS_SPECIFIC_LIBS}
  )

