# - Spire package management module
# Handles setting up Spire and all of its extensions in hopefully the most 
# transparent way possible. This module has been heavily infulenced by
# ExternalProject.cmake.
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
#    [MODULE_DIR dir]             # Module directory. Preferably outside of where cleaning happens.
#    [SHADER_DIR dir]             # Shader directory into which shaders will be copied. If present, shaders will be copied to this directory.
#    [ASSET_DIR dir]              # Asset directory into which assets will be copied. If present, assets will be copied to this directory.
#    )
#
# Spire extensions:
#  Spire_AddExtension(<name>      # Target name that will be constructed for this extension.
#
# TODO: Should have some transparent way of copying shaders and assets. Some function
# that ends up copying all of the assets and shaders into the 'correct' output
# directory (determining what the correct output directory is, may be a harder
# thing to do).
#
# Also remember: you will probably want to use add_dependencies with the target
# name.

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


# See: http://stackoverflow.com/questions/7747857/in-cmake-how-do-i-work-around-the-debug-and-release-directories-visual-studio-2
function(_spm_build_target_output_dirs parent_var_to_update output_dir)

  set(output)
  set(outputs $output "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY:STRING=${output_dir}")
  set(outputs $output "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY:STRING=${output_dir}")
  set(outputs $output "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY:STRING=${output_dir}")

  # Second, for multi-config builds (e.g. msvc)
  foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(outputs $output "-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}:STRING=${output_dir}")
    set(outputs $output "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG}:STRING=${output_dir}")
    set(outputs $output "-DCMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG}:STRING=${output_dir}")
  endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

  set(${parent_var_to_update} ${outputs} PARENT_SCOPE)

endfunction()


# 'name' - Name of the target that will be created.
# This function will define or add to the following variables in the parent's
# namespace.
# 
#  SPIRE_INCLUDE_DIR     - All the spire include directories, including extensions.
#  SPIRE_LIBRARY         - All libraries to link against, including extensions.
#
function(Spire_AddCore name)
  # Parse all function arguments into our namespace prepended with _SPM_.
  _spm_parse_arguments(Spire_AddCore _SPM_ "${ARGN}")

  # Setup any defaults that the user provided.
  if (_SPM_PREFIX)
    set(_ep_prefix "PREFIX" "${_SPM_PREFIX}")
  else()
    set(_ep_prefix "PREFIX" "${CMAKE_CURRENT_BINARY_DIR}/spire-core")
    # We also set the _SPM_PREFIX variable in this case since we use it below.
    set(_SPM_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/spire-core")
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

  # All the following 3 lines do is construct a series of values that will go
  # into the CMAKE_ARGS key in ExternalProject_Add. These are a series
  # binary of output directories. We want a central location for everything
  # so we can keep track of the binaries.
  set(_SPM_BASE_OUTPUT_DIR "${_SPM_PREFIX}/spire_modules")
  set(_SPM_CORE_OUTPUT_DIR "${_SPM_BASE_OUTPUT_DIR}/spire_core")
  _spm_build_target_output_dirs(_ep_spire_output_dirs ${_SPM_CORE_OUTPUT_DIR})

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
      ${_ep_spire_output_dirs}
    )

  # This target property is used to place compiled modules where they belong.
  set_target_properties(${name} PROPERTIES SPIRE_MODULE_OUTPUT_DIRECTORY "${_SPM_BASE_OUTPUT_DIR}")

  # Library path for the core module.
  set(SPIRE_LIBRARIES ${SPIRE_LIBRARIES} Spire PARENT_SCOPE)
  set(SPIRE_LIBRARY_DIRS ${SPIRE_LIBRARY_DIRS} "${_SPM_CORE_OUTPUT_DIR}" PARENT_SCOPE)

  # Retrieving properties from external projects will retrieve their fully
  # initialized values (including if any defaults were set).
  # ExternalProject_Get_Property stores the result in our function scope,
  # under the name GIT_REPOSITORY.
  ExternalProject_Get_Property(${name} GIT_REPOSITORY)
  ExternalProject_Get_Property(${name} SOURCE_DIR)
  ExternalProject_Get_Property(${name} BINARY_DIR)
  ExternalProject_Get_Property(${name} INSTALL_DIR)

  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} "${SOURCE_DIR}")
  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} "${SOURCE_DIR}/Spire/3rdParty/glm")
  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} "${SOURCE_DIR}/Spire/3rdParty/glew/include")
  set(SPIRE_INCLUDE_DIRS ${SPIRE_INCLUDE_DIRS} PARENT_SCOPE)

endfunction()

# Extensions are build with using the provided CMakeLists.txt, but the output
# directories of the extensions are modified such that they end up in a
# unified area. Also, extensions are linked against the already pre-existing 
# spire library. Either dynamically or statically.
function (Spire_AddExtension spire_core name)

endfunction()

