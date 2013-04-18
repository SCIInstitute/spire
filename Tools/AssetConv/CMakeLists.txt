cmake_minimum_required (VERSION 2.8)
project (AssetConverter)
set (${PROJECT_NAME}_VERSION_MAJOR 1)
set (${PROJECT_NAME}_VERSION_MINOR 0)

message("${PROJECT_NAME} Version: " ${${PROJECT_NAME}_VERSION_MAJOR} "." ${${PROJECT_NAME}_VERSION_MINOR})

# Common Variables 
set (TCLAP_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/../3rdParty/tclap/include")

# Libraries / Includes / Definitions
include_directories("${TCLAP_INCLUDE_DIR}")
add_definitions(-DTARGET_OS_MAC)

message(STATUS ${PROJECT_BINARY_DIR})

# All source files in the current directory.
file(GLOB Source_ProjectSourceDir
  "*.h"
  "*.cpp"
  )

########################################################################
# BOOST

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON) 
set(Boost_USE_STATIC_RUNTIME OFF)
FIND_PACKAGE( Boost 1.50.0 COMPONENTS filesystem system REQUIRED )
INCLUDE_DIRECTORIES( ${Boost_INCLUDE_DIR} )

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif ()

################################################################################
# Amalgamation
################################################################################
set(EXE_NAME ${PROJECT_NAME}_r)

add_executable( ${EXE_NAME}
                ${Source_ProjectSourceDir}
                )
if (APPLE)
  target_link_libraries(${EXE_NAME} assimp IL ILU jpeg tiff jasper ${Boost_LIBRARIES})
endif()

