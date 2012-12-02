# Builds all spire related subprojects.
# Includes batch renderer and standalone viewer.

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(SpireViewer)

########################################################################
# Find and configure Qt

set(QT_MIN_VERSION "4.8.0")
find_package(Qt4 REQUIRED)

if (QT_FOUND)
  include_directories(${QT_INCLUDE_DIR})
else ()
  message(FATAL_ERROR "QT 4.7 or greater is required to build SCIRun5Prototype")
endif ()

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif ()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# Since moc and UIC happen outside of the source tree, we need to
# add the binary directory to the include path.
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

########################################################################
# Setup source

file (GLOB Sources "*.cpp")
set (MocHeaders MainWindow.h)
set (Forms MainWindow.ui) 

QT4_WRAP_CPP(MocSource ${MocHeaders})
QT4_WRAP_UI(UISource ${Forms})

########################################################################
# Setup executable

add_executable(spireview ${Sources} ${MocSource} ${UISource})
target_link_libraries(spireview ${QT_LIBRARIES})

