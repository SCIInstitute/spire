# Builds all spire related subprojects.
# Includes batch renderer and standalone viewer.

CMAKE_MINIMUM_REQUIRED(VERSION 2.8 FATAL_ERROR)

PROJECT(BuildAll)

########################################################################
# Find and configure Git

FIND_PACKAGE(Git)
IF(NOT GIT_FOUND)
  MESSAGE(FATAL_ERROR "Git not found.")
ENDIF()

########################################################################
# Configure Spire external project
# NOTE: We should eventually ONLY use the renderer, not all of SCIRUN.

ExternalProject_Add(Spire


