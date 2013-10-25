cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(SpireViewer)

#-----------------------------------------------------------------------
# Configure OpenGL
#-----------------------------------------------------------------------
find_package(OpenGL REQUIRED)
set(QT_USE_QTOPENGL TRUE)

#-----------------------------------------------------------------------
# CMake Options
#-----------------------------------------------------------------------

option(USE_SCIRUN_INTERFACE "Use the SCIRun interface module." ON)
option(USE_LOCAL_SPIRE "Will use a local version of spire instead of downloading from its git repo." ON)
option(USE_LOCAL_SPIRE_SCIRUN "Will use a local version of the spire-scirun module instead of downloading from its git repo." OFF)

#------------------------------------------------------------------------------
# Required CPM Setup - See: http://github.com/iauns/cpm
#------------------------------------------------------------------------------
set (CPM_DIR "${CMAKE_CURRENT_BINARY_DIR}/cpm-packages" CACHE TYPE STRING)
if (${CPM_DIR} MATCHES "${CMAKE_CURRENT_BINARY_DIR}")
  message("NOTE: Placing CPM in the binary directory is not recommended.")
  message("      Place CPM alongside the binary directory so that you don't need to")
  message("      recompile your modules everytime you clean your project.")
  message("      Use the CPM_DIR variable to set the CPM directory.")
endif ()

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

# ++ MODULE: Spire
# Include spire first, then the scirun interface. USE_STD_THREADS will be
# exposed to spire. Threads will be removed from spire in the near future,
# so this option will go away.
option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." ON)
CPM_AddModule("spire"
  GIT_REPOSITORY "https://github.com/SCIInstitute/spire"
  GIT_TAG "origin/concurrent"
  )

# ++ MODULE: Spire SCIRun interface
if (USE_SCIRUN_INTERFACE)
  add_definitions(-DUSE_SCIRUN_INTERFACE)
  # Set various options for spire-scirun. In this case, the shader and
  # asset directories.
  set(SHADER_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Shaders)
  set(ASSET_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/Assets)
  CPM_AddModule("spire_scirun"
    GIT_REPOSITORY "https://github.com/iauns/spire-scirun"
    GIT_TAG "origin/master"
    )
  set(SHADER_OUTPUT_DIR)
  set(ASSET_OUTPUT_DIR)
endif()

CPM_Finish()

#-----------------------------------------------------------------------
# Compiler settings
#-----------------------------------------------------------------------

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Werror -Wshadow")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif ()

# Since moc and UIC happen outside of the source tree, we need to
# add the binary directory to the include path.
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

#-----------------------------------------------------------------------
# Find and configure Qt
#-----------------------------------------------------------------------

set(QT_MIN_VERSION "4.8.0")
find_package(Qt4 REQUIRED)

if (QT_FOUND)
  include_directories(SYSTEM "${QT_INCLUDE_DIR}")
else ()
  message(FATAL_ERROR "QT 4.8 or greater is required to build Spire Viewer")
endif ()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

#-----------------------------------------------------------------------
# Setup source
#-----------------------------------------------------------------------

set(View_Common_Sources GLContext.cpp GLContext.h MainWindow.cpp MainWindow.h main.cpp)
if(USE_SCIRUN_INTERFACE)
  set(View_Specific_Sources GLWidgetSCIRun.cpp GLWidgetSCIRun.h)
  set(MocHeaders MainWindow.h GLWidgetSCIRun.h)
else()
  set(View_Specific_Sources GLWidget.cpp GLWidget.h)
  set(MocHeaders MainWindow.h GLWidget.h)
endif()

set (Forms MainWindow.ui) 

QT4_WRAP_CPP(MocSource ${MocHeaders})
QT4_WRAP_UI(UISource ${Forms})

#-----------------------------------------------------------------------
# Copy shaders to the binary directory
#-----------------------------------------------------------------------
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Shaders
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

#-----------------------------------------------------------------------
# Setup executable
#-----------------------------------------------------------------------

add_executable(spireview 
  ${View_Common_Sources}
  ${View_Specific_Sources}
  ${MocSource} 
  ${UISource})

target_link_libraries(spireview 
  ${CPM_LIBRARIES} 
  ${QT_OPENGL_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${QT_LIBRARIES})

