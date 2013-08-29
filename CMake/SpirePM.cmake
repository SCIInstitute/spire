# - Spire package management module
# Handles setting up Spire and all of its extensions in, hopefully, the most 
# transparent way possible in CMake. Heavily infulenced by ExternalProject.cmake.
# Some 
#
# Should have some transparent way of copying shaders and assets. Some function
# that ends up copying all of the assets and shaders into the 'correct' output
# directory (determining what the correct output directory is, may be a harder
# thing to do).
#
# Also remember: you will probably want to use add_dependencies with the target
# name.
# 
# The Spire_AddCore and Spire_AddExtension functions add the following
# variables to the PARENT_SCOPE namespace:
# 
#  SPIRE_INCLUDE_DIRS   - All the spire include directories.
#  SPIRE_LIBRARIES      - All libraries to link against.
#
# And these variables are only added by Spire_AddExtension:
#
#  SPIRE_SHADER_DIRS    - All shader asset directories.
#  SPIRE_ASSET_DIRS     - All asset directories.
#
# Spire core:
#  Spire_AddCore(<name>           # Target name that will be constructed for spire.
#    [PREFIX dir]                 # Same as ExternalProject_Add's PREFIX.
#    [SOURCE_DIR dir]             # Same as ExternalProject_Add's SOURCE_DIR.
#    [BINARY_DIR dir]             # Same as ExternalProject_Add's BINARY_DIR .
#    [GIT_TAG tag]                # Same as ExternalProject_Add's GIT_TAG
#    [GIT_REPOSITORY repo]        # Same as ExternalProject_Add's GIT_REPOSITORY.
#    [USE_STD_THREADS truth]      # Set to 'ON' if you want to use spire threads.
#    [USE_SHARED_LIB truth]       # Set to 'ON' if you want build a shared library.
#    )
#
# Spire extensions:
#  Spire_AddExtension(<name>      # Target name that will be constructed for this extension.

#-------------------------------------------------------------------------------
# Pre-compute a regex to match documented keywords for each command.
#-------------------------------------------------------------------------------
# This code parses the *current* file and extracts parameter key words from the
# documentation given above. It will match "# ... [...] # ..." style statements,
# or "#  <funcname>(" style statements.
# This code was pretty much lifted directly from KitWare's ExternalProject.cmake,
# but then I documented what it's doing. It's not exactly straight forward.

# Based on the current line in *this* file (SpirePM.cmake), we calc the number
# of lines the documentation header consumes. Including this comment, that is
# 12 lines upwards.
math(EXPR _spm_documentation_line_count "${CMAKE_CURRENT_LIST_LINE} - 13")

# Run a regex to extract parameter names from the *this* file (SpirePM.cmake).
# Stuff the results into 'lines'.
file(STRINGS "${CMAKE_CURRENT_LIST_FILE}" lines
     LIMIT_COUNT ${_spm_documentation_line_count}
     REGEX "^#  (  \\[[A-Z0-9_]+ [^]]*\\] +#.*$|[A-Za-z0-9_]+\\()")

# Iterate over the results we obtained 
foreach(line IN LISTS lines)
  # Check to see if we have found a function which is two spaces followed by
  # any number of alphanumeric chararcters followed by a '('.
  if("${line}" MATCHES "^#  [A-Za-z0-9_]+\\(")

    # Are we already parsing a function?
    if(_spm_func)
      # We are parsing a function, save the current list of keywords in 
      # _spm_keywords_<function_name> in preparation to parse a new function.
      set(_spm_keywords_${_spm_func} "${_spm_keywords_${_spm_func}})$")
    endif()

    # Note that _spm_func gets *set* HERE. See 'cmake --help-command string'.
    # In this case, we are extracting the function's name into _spm_func.
    string(REGEX REPLACE "^#  ([A-Za-z0-9_]+)\\(.*" "\\1" _spm_func "${line}")

    #message("function [${_spm_func}]")

    # Clear vars (we will be building a REGEX in _spm_keywords, hence
    # the ^(. _spm_keyword_sep is only use to inject a separator at appropriate
    # places while we are building the regex. In essence, we are skipping the
    # first '|' that would usually be inserted.
    set(_spm_keywords_${_spm_func} "^(")
    set(_spm_keyword_sep)
  else()
    # Otherwise we must be parsing a parameter of the function. Extract the name
    # of the parameter into _spm_key
    string(REGEX REPLACE "^#    \\[([A-Z0-9_]+) .*" "\\1" _spm_key "${line}")
    # Syntax highlighting gets a little wonky around this regex, need this - "

    #message("  keyword [${_spm_key}]")

    set(_spm_keywords_${_spm_func}
      "${_spm_keywords_${_spm_func}}${_spm_keyword_sep}${_spm_key}")
    set(_spm_keyword_sep "|")
  endif()
endforeach()
# Duplicate of the 'Are we already parsing a function?' code above.
# Just completes the regex.
if(_spm_func)
  set(_spm_keywords_${_spm_func} "${_spm_keywords_${_spm_func}})$")
endif()

# Include external project
include(ExternalProject)

# Function for parsing arguments and values coming into the specified function
# name 'f'. 'name' is the target name. 'ns' (namespace) is a value prepended
# onto the key name before being added to the target namespace. 'args' list of
# arguments to process.
function(_spm_parse_arguments f ns args)
  # Transfer the arguments to this function into target properties for the new
  # custom target we just added so that we can set up all the build steps
  # correctly based on target properties.
  #
  # We loop through ARGN and consider the namespace starting with an upper-case
  # letter followed by at least two more upper-case letters, numbers or
  # underscores to be keywords.
  set(key)

  foreach(arg IN LISTS args)
    set(is_value 1)

    # Check to see if we have a keyword. Otherwise, we will have a value
    # associated with a keyword. Confirm that the arg doesn't match a few
    # common exceptions.
    if(arg MATCHES "^[A-Z][A-Z0-9_][A-Z0-9_]+$" AND
        NOT ((arg STREQUAL "${key}") AND (key STREQUAL "COMMAND")) AND
        NOT arg MATCHES "^(TRUE|FALSE)$")

      # Now check to see if the argument is in our list of approved keywords.
      # If is, then make sure we don't treat it as a value.
      if(_spm_keywords_${f} AND arg MATCHES "${_spm_keywords_${f}}")
        set(is_value 0)
      endif()

    endif()

    if(is_value)
      if(key)
        # We have a key / value pair. Set the appropriate property.
        if(NOT arg STREQUAL "")
          # Set the variable in both scopes so we can test for existance
          # and update as needed.
          set(${ns}${key} "${arg}")
          set(${ns}${key} "${arg}" PARENT_SCOPE)
          message("Set ${ns}${key} to ${arg}")
        else()
          if (${ns}${key})
            # If we already have a value for this key, generated a semi-colon
            # separated list.
            set(value ${${ns}${key}})
            set(${ns}${key} "${value};${arg}")
            set(${ns}${key} "${value};${arg}" PARENT_SCOPE)
            message("Set2 ${ns}${key} to ${value};${arg}")
          else()
            set(${ns}${key} "${arg}")
            set(${ns}${key} "${arg}" PARENT_SCOPE)
          endif()
        endif()
      else()
        # Missing Keyword
        message(AUTHOR_WARNING "value '${arg}' with no previous keyword in ${f}")
      endif()
    else()
      # Set the key to use in the next iteration.
      set(key "${arg}")
    endif()
  endforeach()
endfunction()

# 'name' - Name of the target that will be created.
# This function will define or add to the following variables in the parent's
# namespace.
# 
#  SPIRE_INCLUDE_DIR     - All the spire include directories, including extensions.
#  SPIRE_LIBRARY         - All libraries to link against, including extensions.
#
function(Spire_AddCore name)
  # Create the external project before parsing our arguments (this will create
  # a target with ${name}).

  # NOTE: If the PREFIX option is given to ExternalProject_Add, or EP_PREFIX
  # directory property is set, then an external project is built and installed
  # under the specified prefix:
  #   TMP_DIR      = <prefix>/tmp
  #   STAMP_DIR    = <prefix>/src/<name>-stamp
  #   DOWNLOAD_DIR = <prefix>/src
  #   SOURCE_DIR   = <prefix>/src/<name>
  #   BINARY_DIR   = <prefix>/src/<name>-build
  #   INSTALL_DIR  = <prefix>
  # Otherwise, if the EP_BASE directory property is set then components
  # of an external project are stored under the specified base:
  #   TMP_DIR      = <base>/tmp/<name>
  #   STAMP_DIR    = <base>/Stamp/<name>
  #   DOWNLOAD_DIR = <base>/Download/<name>
  #   SOURCE_DIR   = <base>/Source/<name>
  #   BINARY_DIR   = <base>/Build/<name>
  #   INSTALL_DIR  = <base>/Install/<name>
  # If no PREFIX, EP_PREFIX, or EP_BASE is specified then the default
  # is to set PREFIX to "<name>-prefix".
  # Relative paths are interpreted with respect to the build directory
  # corresponding to the source directory in which ExternalProject_Add is
  # invoked.

  # If SOURCE_DIR is explicitly set to an existing directory the project
  # will be built from it.
  # Otherwise a download step must be specified using one of the
  # DOWNLOAD_COMMAND, CVS_*, SVN_*, or URL options.
  # The URL option may refer locally to a directory or source tarball,
  # or refer to a remote tarball (e.g. http://.../src.tgz).

  # Parse all function arguments into GLOBAL namespace with _SPM
  _spm_parse_arguments(Spire_AddCore _SPM_ "${ARGN}")

  # Set prefix according to user, or use spire-core.
  # Note that setting the variables this way, with two values, generates a
  # semi colon delimited list as a string. So:
  # set(_ep_prefix "PREFIX" "spire-core") is the same as
  # set(_ep_prefix "PREFIX;spire-core")
  # And behaves just as you would expect when used in the ExternalProject_Add
  # function.
  if (_SPM_PREFIX)
    set(_ep_prefix "PREFIX" "${_SPM_PREFIX}")
  else()
    set(_ep_prefix "PREFIX" "spire-core")
  endif()

  if (_SPM_SOURCE_DIR)
    set(_ep_source_dir "SOURCE_DIR" "${_SPM_SOURCE_DIR}")
  else()
    set(_ep_source_dir "SOURCE_DIR")
  endif()

  if (_SPM_GIT_TAG)
    set(_ep_git_tag "GIT_TAG" ${_SPM_GIT_TAG})
  else()
    set(_ep_git_tag "GIT_TAG" "master")
  endif()

  if (_SPM_GIT_REPOSITORY)
    set(_ep_git_repo "GIT_REPOSITORY" ${_SPM_GIT_REPOSITORY})
  else()
    set(_ep_git_repo "GIT_REPOSITORY" "https://github.com/SCIInstitute/spire.git")
  endif()

  if (_SPM_BINARY_DIR)
    set(_ep_bin_dir "BINARY_DIR" $_SPM_BINARY_DIR)
  endif()

  if (_SPM_USE_STD_THREADS)
    set(_ep_spire_use_threads "-DUSE_STD_THREADS:BOOL=${_SPM_USE_STD_THREADS}")
  else()
    set(_ep_spire_use_threads "-DUSE_STD_THREADS:BOOL=ON")
  endif()

  if (_SPM_USE_SHARED_LIB)
    set(_ep_spire_use_shared "-DBUILD_SHARED_LIBS:BOOL=${_SPM_USE_SHARED_LIB}")
  else()
    set(_ep_spire_use_shared "-DBUILD_SHARED_LIBS:BOOL=OFF")
  endif()

  # All parent scope variables are copied into our scope.
  #message("CMake build type from spire func: ${CMAKE_BUILD_TYPE}")

  ExternalProject_Add(${name}
    ${_ep_prefix}
    ${_ep_source_dir}
    ${_ep_git_repo}
    ${_ep_git_tag}
    ${_ep_bin_dir}
    INSTALL_COMMAND ""
    CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}
      ${_ep_spire_use_shared}
      ${_ep_spire_use_threads}
    )

  # Retrieving properties from external projects will retrieve their fully
  # initialized values (including if any defaults were set).
  # ExternalProject_Get_Property stores the result in our function scope,
  # under the name GIT_REPOSITORY.
  ExternalProject_Get_Property(${name} GIT_REPOSITORY)
  ExternalProject_Get_Property(${name} SOURCE_DIR)
  ExternalProject_Get_Property(${name} BINARY_DIR)

  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} "${SOURCE_DIR}")
  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} "${SOURCE_DIR}/Spire/3rdParty/glm")
  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} "${SOURCE_DIR}/Spire/3rdParty/glew/include")
  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} PARENT_SCOPE)

  # TODO: Figure out either the install step, or the correct location to
  # libraries.
  set(SPIRE_LIBRARY_DIRS ${SPIRE_LIBRARY_DIRS} "${BINARY_DIR}" PARENT_SCOPE)

  set(SPIRE_LIBRARIES ${SPIRE_LIBRARIES} Spire PARENT_SCOPE)

  # Set properties in the ${name} target. These range from the include
  # directories to the output binaries.
endfunction()

function (Spire_AddExtension)

endfunction()

