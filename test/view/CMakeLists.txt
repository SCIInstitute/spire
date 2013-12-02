cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(SpireViewer)

#-----------------------------------------------------------------------
# Configure OpenGL
#-----------------------------------------------------------------------
find_package(OpenGL REQUIRED)
set(QT_USE_QTOPENGL TRUE)

#-----------------------------------------------------------------------
# Compiler settings (C++11)
#-----------------------------------------------------------------------
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

# ++ MODULE: Spire
CPM_AddModule("spire"
  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  SOURCE_GHOST_GIT_REPO "https://github.com/SCIInstitute/spire"
  SOURCE_GHOST_GIT_TAG "origin/master")

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

set(QT_MIN_VERSION "4.7.0")
find_package(Qt4 REQUIRED)

if (QT_FOUND)
  include_directories(SYSTEM "${QT_INCLUDE_DIR}")
else ()
  message(FATAL_ERROR "QT 4.7 or greater is required to build Spire Viewer")
endif ()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

#-----------------------------------------------------------------------
# Setup source
#-----------------------------------------------------------------------

set(View_Common_Sources GLContext.cpp GLContext.h MainWindow.cpp MainWindow.h main.cpp)
set(View_Specific_Sources GLWidget.cpp GLWidget.h)
set(MocHeaders MainWindow.h GLWidget.h)

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

