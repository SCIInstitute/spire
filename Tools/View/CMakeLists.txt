cmake_minimum_required(VERSION 2.8.11 FATAL_ERROR)
project(SpireViewer)

#-----------------------------------------------------------------------
# Configure OpenGL
#-----------------------------------------------------------------------
find_package(OpenGL REQUIRED)
set (QT_USE_QTOPENGL TRUE)

#-----------------------------------------------------------------------
# Common Variables
#-----------------------------------------------------------------------

set (BASE_SPIRE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

#-----------------------------------------------------------------------
# Spire package setup
#-----------------------------------------------------------------------

# Ensure CMAKE_MODULE_PATH includes path to Spire's SpirePM.cmake.
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake)
include(SpirePM)

# Since we already have spire in the source chain, we won't download it.
#Spire_AddCore(Spire
#  SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# Add core spire library locally.
# This function, alongside Spire_AddExtension, populate the SPIRE_INCLUDE_DIR
# and SPIRE_LIBRARY paths. Extension may also populate SPIRE_SHADER_DIR and
# SPIRE_ASSET_DIR so that you can copy assets and the like.
# We use the default settings with the target name of 'Spire'.
Spire_AddCore(Spire)

## TESTING EXTENSIONS:
## Add the SCIRun extension to spire while also specifying the version of
## spire-scirun to use. The version is pulled as a git-tag, so it should be
## present as a tag in the git repo.
#Spire_AddExtension(SCIRun "https://github.com/iauns/spire-scirun" "0.1.0")

# Include spire directories that were place in our scope by the Spire_AddCore
# and Spire_AddExtension functions.
include_directories()

#-----------------------------------------------------------------------
# Compiler settings
#-----------------------------------------------------------------------

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif ()

if (APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wall -Wextra -Werror -Wconversion -Wshadow")
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
# Spire Extensions
#-----------------------------------------------------------------------

# Extensions include directory.
include_directories(SYSTEM "${BASE_SPIRE_DIR}/Extensions" )

#-----------------------------------------------------------------------
# Eigen
#-----------------------------------------------------------------------

# Eigen include directory.
include_directories(SYSTEM "${BASE_SPIRE_DIR}/Spire/3rdParty/eigen" )

#-----------------------------------------------------------------------
# GLM
#-----------------------------------------------------------------------

# GLM include directory.
include_directories(SYSTEM "${BASE_SPIRE_DIR}/Spire/3rdParty/glm" )

#-----------------------------------------------------------------------
# Setup source
#-----------------------------------------------------------------------

set(View_Common_Sources GLContext.cpp GLContext.h MainWindow.cpp MainWindow.h main.cpp)
if(USE_SCIRUN_INTERFACE)
  set(View_Specific_Sources GLWidgetSCIRun.cpp GLWidgetSCIRun.h)
  set(MocHeaders MainWindow.h GLWidgetSCIRun.h)

  file (GLOB Source_Extensions
    "${BASE_SPIRE_DIR}/Extensions/SpireExt/SCIRun/*.cpp"
    "${BASE_SPIRE_DIR}/Extensions/SpireExt/SCIRun/*.h"
    )
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

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Assets/
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Assets
     FILE_PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ
     DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                           GROUP_READ
                           WORLD_READ WORLD_EXECUTE)

#-----------------------------------------------------------------------
# Setup executable
#-----------------------------------------------------------------------

add_executable(spireview 
  ${Source_Extensions}
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

