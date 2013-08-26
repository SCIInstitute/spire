
# Dummy code until we get the tuvok library importd and running smoothly.

########################################################################
# Tuvok

if(USE_TUVOK)
  # FIND QT -- Required for compiling tuvok...
  # Tuvok's build system needs to be updated in the future so that it can
  # use builders other than QT. There is no reason to have a QT dependency
  # just for generating a makefile for Tuvok.
  set(QT_MIN_VERSION "4.8.0")
  find_package(Qt4 REQUIRED)

  if(QT_FOUND)
  else()
    message(FATAL_ERROR "QT 4.9 or greater is required to build Spire (for Tuvok)")
  endif()

  # Git (for checking out external projects).
  find_package(Git)
  if(NOT GIT_FOUND)
    MESSAGE(FATAL_ERROR "Git not found.")
  endif()

  # Svn
  include(FindSubversion)

  # Tuvok
  # See: http://cmake.org/cmake/help/v2.8.8/cmake.html#module:ExternalProject
  set(TuvokSourceDir "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/Tuvok")
  set(TuvokBinaryDir "${CMAKE_CURRENT_BINARY_DIR}/3rdParty/Tuvok")

  if(UNIX)

    if(APPLE)
      # TODO: Get rid of unsupported macx/clang when we switch to qt5.
      # Note: There are no quotes around the command. If there were,
      #       sh would interpret it as a file
      set(TuvokConfigureStep ${QT_QMAKE_EXECUTABLE} -spec unsupported/macx-clang QMAKE_CONFIG+='release' QMAKE_CFLAGS='-O2' QMAKE_CXXFLAGS='-O2' QMAKE_LFLAGS='' -recursive ${TuvokSourceDir}/Tuvok.pro)
      set(TuvokMakeStep make -j2)
    else(APPLE)

    endif(APPLE)

    # Ensure we can link against libTuvok.a
    set(TUVOK_LIB ${TuvokBinaryDir}/Build/libTuvok.a)

    # Note: The build/configure command will vary based on platform!
    # Specify appropriate Tuvok SVN revision when we get a stable version.
    # We set UPDATE_COMMAND to "" because we don't want to svn update everytime
    # we build the project.
    ExternalProject_Add(ThirdParty_Tuvok
      PREFIX ${TuvokBinaryDir}
      SVN_REPOSITORY "https://gforge.sci.utah.edu/svn/Tuvok"
      SVN_TRUST_CERT 1
      DOWNLOAD_DIR "${CMAKE_CURRENT_BINARY_DIR}/download/Tuvok"
      UPDATE_COMMAND ""
      SOURCE_DIR ${TuvokSourceDir}
      BINARY_DIR ${TuvokBinaryDir}
      CONFIGURE_COMMAND ${TuvokConfigureStep}
      BUILD_COMMAND ${TuvokMakeStep}
      )

  else(UNIX)
    if(WIN32)

      # TODO Figure out how to build Tuvok with the sln files on windows OR
      #      fix tuvok so that it uses qmake to generate the sln files, just
      #      like unix.

    else(WIN32)
      MESSAGE(FATAL_ERROR "Unknown platform.")
    endif(WIN32)
  endif(UNIX)
endif(USE_TUVOK)
