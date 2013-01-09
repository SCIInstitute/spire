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
  include_directories(${QT_INCLUDE_DIR})
else ()
  message(FATAL_ERROR "QT 4.7 or greater is required to build Spire Viewer")
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
add_subdirectory(${BASE_SPIRE_DIR}/Spire ${BASE_SPIRE_DIR}/bin/spire)

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
include_directories("${CMAKE_CURRENT_BINARY_DIR}" "${BASE_SPIRE_DIR}" )

########################################################################
# Setup source

file (GLOB Sources "*.cpp")
set (MocHeaders MainWindow.h)
set (Forms MainWindow.ui) 

QT4_WRAP_CPP(MocSource ${MocHeaders})
QT4_WRAP_UI(UISource ${Forms})

########################################################################
# Copy shaders to the target directory
configure_file(${BASE_SPIRE_DIR}/Spire/Shaders/UniformColor.vs ${CMAKE_CURRENT_BINARY_DIR}/Shaders/UniformColor.vs)
configure_file(${BASE_SPIRE_DIR}/Spire/Shaders/UniformColor.fs ${CMAKE_CURRENT_BINARY_DIR}/Shaders/UniformColor.fs)

########################################################################
# Setup executable

add_executable(spireview 
  ${Sources} 
  ${MocSource} 
  ${UISource})

target_link_libraries(spireview 
  Spire 
  ${QT_OPENGL_LIBRARY}
  ${OPENGL_LIBRARIES}
  ${QT_LIBRARIES})

