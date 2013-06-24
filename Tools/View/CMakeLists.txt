# Build the minimal Spire viewer.

########################################################################
# CMake version and project name

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(SpireViewer)

########################################################################
# Find and configure Qt

set(QT_MIN_VERSION "4.8.0")
find_package(Qt4 REQUIRED)

if (QT_FOUND)
  include_directories("${QT_INCLUDE_DIR}")
else ()
  message(FATAL_ERROR "QT 4.8 or greater is required to build Spire Viewer")
endif ()

########################################################################
# Configure OpenGL
find_package(OpenGL REQUIRED)
set (QT_USE_QTOPENGL TRUE)

########################################################################
# Common Variables

set (BASE_SPIRE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

########################################################################
# Required libraries

# The 'spire' library.
add_subdirectory(${BASE_SPIRE_DIR}/Spire ${CMAKE_CURRENT_BINARY_DIR}/spire)

# Pull USE_STD_THREADS from the spire directory
get_directory_property(USE_STD_THREADS DIRECTORY ${BASE_SPIRE_DIR}/Spire DEFINITION USE_STD_THREADS)
if(USE_STD_THREADS)
  add_definitions(-DSPIRE_USE_STD_THREADS)
endif()

get_directory_property(USI_SCIRUN_INTERFACE DIRECTORY ${BASE_SPIRE_DIR}/Spire DEFINITION USE_SCIRUN_INTERFACE)
if (USE_SCIRUN_INTERFACE)
  add_definitions(-DSPIRE_USE_SCIRUN_INTERFACE)
endif()

########################################################################
# Eigen

# Eigen include directory.
INCLUDE_DIRECTORIES( "${BASE_SPIRE_DIR}/Spire/3rdParty/eigen" )

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++")
endif ()

include(${QT_USE_FILE})
add_definitions(${QT_DEFINITIONS})

# Since moc and UIC happen outside of the source tree, we need to
# add the binary directory to the include path.
include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${BASE_SPIRE_DIR}" )

########################################################################
# Setup source

set(View_Common_Sources GLContext.cpp GLContext.h MainWindow.cpp MainWindow.h main.cpp)

# An example where globbing isn't so good =(
if(USE_SCIRUN_INTERFACE)
  set(View_Specific_Sources GLWidgetSCIRun.cpp GLWidgetSCIRun.h)
  set (MocHeaders MainWindow.h GLWidgetSCIRun.h)
else()
  set(View_Specific_Sources GLWidget.cpp GLWidget.h)
  set (MocHeaders MainWindow.h GLWidget.h)
endif()

set (Forms MainWindow.ui) 

QT4_WRAP_CPP(MocSource ${MocHeaders})
QT4_WRAP_UI(UISource ${Forms})

########################################################################
# Copy shaders to the target directory
install(DIRECTORY ${BASE_SPIRE_DIR}/Spire/Shaders
        DESTINATION ${CMAKE_INSTALL_DIR}/
        FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
        DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                              GROUP_READ
                              WORLD_READ WORLD_EXECUTE)

# Copy shaders.
file(COPY ${BASE_SPIRE_DIR}/Spire/Shaders/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

########################################################################
# Setup executable

add_executable(spireview 
  ${View_Common_Sources}
  ${View_Specific_Sources}
  ${MocSource} 
  ${UISource})

add_dependencies(spireview Spire)

# Example of extracting a variable from another cmake dir:
#get_directory_property(SPIRE_EXTRA_LIBS DIRECTORY ${BASE_SPIRE_DIR}/Spire DEFINITION OS_SPECIFIC_LIBS)
#message( Dir: ${BASE_SPIRE_DIR}/Spire)
#message( Libs: ${SPIRE_EXTRA_LIBS})

target_link_libraries(spireview 
  Spire 
  ${QT_OPENGL_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${QT_LIBRARIES})

