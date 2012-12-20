# Builds the spire library and all of its dependencies.
#set (RENDERER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

########################################################################
# OpenGL configuration

# Find OpenGL
find_package(OpenGL REQUIRED)

# Include OpenGL headers
include_directories(${OPENGL_INCLUDE_DIRS})

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
endif (UNIX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

########################################################################
# ThirdParty libraries

# Git (for checking out external projects).
find_package(Git)
if (NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Git not found.")
endif ()

# Svn
include(FindSubversion)

# Tuvok
# See: http://cmake.org/cmake/help/v2.8.8/cmake.html#module:ExternalProject
#set(TuvokSourceDir "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Tuvok")
#set(TuvokBinaryDir "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/Tuvok")

# Specify appropriate Tuvok SVN revision when we get a stable version.
# BUILD_COMMAND "scripts/oneshot-debug.sh"
#ExternalProject_Add(ThirdParty_Tuvok
#  PREFIX ${TuvokBinaryDir}
#  SVN_REPOSITORY "https://gforge.sci.utah.edu/svn/Tuvok"
#  SVN_TRUST_CERT 1
#  SOURCE_DIR ${TuvokSourceDir}
#  )

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
  "Pipes/*.cpp"
  "Pipes/*.h"
  "Pipes/StuPipe/*.cpp"
  "Pipes/StuPipe/*.h"
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
target_link_libraries(Spire ${OPENGL_LIBRARIES})

