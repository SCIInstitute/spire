# Build the minimal Spire viewer.

########################################################################
# CMake version and project name

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

project(SpireViewer)

########################################################################
# Function
# Use the following function to sort / organize shader files.
# We could easily glob all of the shaders together and iterate over them
# + change the prefix of the files (to the appropriate output dir).

# handleShader(target
# [ALL]
# COMMAND <cmd>
# [ARGUMENTS <arg1> ... <argN>]
# [OUTPUT_DIR <dir>]
# [FORMAT <ext>]
# FILES <source1> ... <sourceN>
# [OUTPUT_FILES <outputVar>]
# )
#function(graphicsMagick Target)
#  PARSE_ARGUMENTS(
#    ARGS
#    "COMMAND;ARGUMENTS;OUTPUT_DIR;FORMAT;FILES;OUTPUT_FILES"
#    "ALL"
#    ${ARGN}
#  )
# 
#  # Argument flags.
# 
#  if (ARGS_ALL)
#    set(ARGS_ALL "ALL")
#  else()
#    set(ARGS_ALL)
#  endif()
# 
#  # Argument options.
# 
#  if (NOT ARGS_COMMAND)
#    message(FATAL_ERROR "'COMMAND' argument is mandatory")
#  endif()
# 
#  if (ARGS_OUTPUT_DIR)
#    file(TO_CMAKE_PATH ${ARGS_OUTPUT_DIR} ARGS_OUTPUT_DIR)
#  endif()
# 
#  foreach(SrcFile ${ARGS_FILES})
#    set(DstFile ${SrcFile})
# 
#    # Change file suffix
#    if (ARGS_FORMAT)
#      get_filename_component(SrcExt ${SrcFile} EXT)
#      string(REPLACE ${SrcExt} ".${ARGS_FORMAT}" DstFile ${SrcFile})
#    endif()
# 
#    # Set destination filename to absolute.
#    if (ARGS_OUTPUT_DIR)
#      get_filename_component(DstFile ${DstFile} NAME)
#      set(DstFile ${ARGS_OUTPUT_DIR}/${DstFile})
#    else()
#      set(DstFile ${CMAKE_CURRENT_BINARY_DIR}/${DstFile})
#    endif()
# 
#    get_filename_component(SrcFileFull ${SrcFile} ABSOLUTE)
# 
#    add_custom_command(
#      OUTPUT ${DstFile}
#      COMMAND ${GRAPHICSMAGICK_EXECUTABLE} ${ARGS_COMMAND} ${SrcFileFull} ${ARGS_ARGUMENTS} ${DstFile}
#      DEPENDS ${SrcFile}
#    )
# 
#    # Command needs existence of target directories.
#    makeTargetDirs(${DstFile})
# 
#    list(APPEND DstFiles ${DstFile})
#  endforeach()
# 
#  if(ARGS_OUTPUT_FILES)
#    set(${ARGS_OUTPUT_FILES} ${DstFiles} PARENT_SCOPE)
#  endif()
# 
#  add_custom_target(${Target} ${ARGS_ALL} DEPENDS ${DstFiles})
#endfunction()

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

########################################################################
# Compiler settings

if (UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
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

file (GLOB Sources "*.cpp")
set (MocHeaders MainWindow.h GLWidget.h)
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
  ${Sources} 
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

