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

#-----------------------------------------------------------------------
# Setup spire
#-----------------------------------------------------------------------

# Ensure CMAKE_MODULE_PATH includes path to Spire's SpirePM.cmake.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../CMake)
include(SpirePM)

# Determine if we should use threading.
set(SPIRE_OPTIONS)
option(USE_STD_THREADS "Use standard library threads. If turned off, no threading support is available." ON)
if (USE_STD_THREADS)
  set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "USE_STD_THREADS;ON")
else()
  set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "USE_STD_THREADS;OFF")
endif()

# Add target shader and asset directories. Be careful of modules overwriting
# your existing shaders.
set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "SHADER_OUTPUT_DIR;${CMAKE_CURRENT_BINARY_DIR}/Shaders")
set(SPIRE_OPTIONS ${SPIRE_OPTIONS} "ASSET_OUTPUT_DIR;${CMAKE_CURRENT_BINARY_DIR}/Assets")

# Add core spire library. Either download it or use a local version.
set(SPIRE_CORE "Spire_Core_EP")
if (USE_LOCAL_SPIRE)
  Spire_AddCore(${SPIRE_CORE} ${SPIRE_OPTIONS}
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")
else()
  Spire_AddCore(${SPIRE_CORE} ${SPIRE_OPTIONS})
endif()

# Add SCIRun interface if requested.
if (USE_SCIRUN_INTERFACE)
  add_definitions(-DUSE_SCIRUN_INTERFACE)

  if (USE_LOCAL_SPIRE_SCIRUN)
    Spire_AddModule(${SPIRE_CORE} SCIRun "https://github.com/iauns/spire-scirun" "master"
      SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../SpireExt/SCIRun")
  else()
    Spire_AddModule(${SPIRE_CORE} SCIRun "https://github.com/iauns/spire-scirun" "master")
  endif()
endif()

# Include / library directories placed in-scope by Spire_AddCore and Spire_AddModule.
link_directories(${SPIRE_LIBRARY_DIRS})
include_directories(${SPIRE_INCLUDE_DIR})
include_directories(${SPIRE_MODULE_INCLUDE_DIRS})
include_directories(SYSTEM ${SPIRE_3RDPARTY_INCLUDE_DIRS})

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

add_dependencies(spireview ${SPIRE_LIBRARIES})

target_link_libraries(spireview 
  ${SPIRE_LIBRARIES} 
  ${QT_OPENGL_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${QT_LIBRARIES})

