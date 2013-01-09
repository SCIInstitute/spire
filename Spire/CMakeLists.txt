# Builds the spire library and all of its dependencies.
#set (RENDERER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")


########################################################################
# CMake configuration

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

INCLUDE(ExternalProject)

########################################################################
# OpenGL configuration

# Find OpenGL
find_package(OpenGL REQUIRED)

########################################################################
# QT configuration

# The only reason we need QT is so that we can compile Tuvok...
# Tuvok's build system needs to be updated in the future so that it can
# use builders other than QT. There is no reason to have a QT dependency
# just for generating a makefile for Tuvok.
SET(QT_MIN_VERSION "4.7.0")
FIND_PACKAGE(Qt4 REQUIRED)

IF(QT_FOUND)
ELSE()
  MESSAGE(FATAL_ERROR "QT 4.7 or greater is required to build Spire (for Tuvok)")
ENDIF()

########################################################################
# Compiler settings

if (UNIX)
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

  if (APPLE)
    add_definitions(-DUSING_OSX)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  else (APPLE)
    add_definitions(-DUSING_LINUX)
  endif (APPLE)

  # Super strict settings.
  #set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
else (UNIX)
  if (WIN32)
    add_definitions(-DUSING_WIN)
  endif (WIN32)
endif (UNIX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/3rdParty ${OPENGL_INCLUDE_DIRS})

########################################################################
# ThirdParty libraries

# TODO: Need a switch to compile without Tuvok.

# Git (for checking out external projects).
find_package(Git)
if (NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Git not found.")
endif ()

# Svn
include(FindSubversion)

# Tuvok
# See: http://cmake.org/cmake/help/v2.8.8/cmake.html#module:ExternalProject
set(TuvokSourceDir "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Tuvok")
set(TuvokBinaryDir "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/Tuvok")

if (UNIX)

  if (APPLE)
    # TODO: Get rid of unsupported macx/clang when we switch to qt5.
    # Note: There are no quotes around the command. If there were,
    #       sh would interpret it as a file
    set(TuvokConfigureStep ${QT_QMAKE_EXECUTABLE} -spec unsupported/macx-clang QMAKE_CONFIG+='release' QMAKE_CFLAGS='-O2' QMAKE_CXXFLAGS='-O2' QMAKE_LFLAGS='' -recursive ${TuvokSourceDir}/Tuvok.pro)
    set(TuvokMakeStep make -j2)
  else (APPLE)

  endif (APPLE)

  # Ensure we can link against libTuvok.a
  set(TUVOK_LIB ${TuvokBinaryDir}/Build/libTuvok.a)

else (UNIX)
  if (WIN32)

  else (WIN32)
    MESSAGE(FATAL_ERROR "Unknown platform.")
  endif (WIN32)
endif (UNIX)

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

########################################################################
# Library setup

add_library(Spire
  ${Source_Root}
  ${Source_High}
  ${Source_Low}
  ${Source_Caching}
  ${Source_Pipes}
  )
add_dependencies(Spire ThirdParty_Tuvok)
target_link_libraries(Spire ${OPENGL_LIBRARIES} ${TUVOK_LIB})

