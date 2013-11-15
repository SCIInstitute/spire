/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.


   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/

#ifndef SPIRE_GL_HEADER_H
#define SPIRE_GL_HEADER_H

#if defined(USE_CORE_PROFILE_3) && defined(USE_CORE_PROFILE_4)
  #error Both USE_CORE_PROFILE_3 and USE_CORE_PROFILE_4 are defined. Ensure that only one is defined.
#endif

/// \todo Add webgl and emscriptem.

// See: http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
// and, http://sourceforge.net/p/predef/wiki/OperatingSystems/
// We provide these definitions in ADDITION to cmake directives to cover 
// extensions that need OpenGL system headers and whatnot.
#ifdef _WIN64
  #define SPIRE_USING_WIN
#elif _WIN32
  #define SPIRE_USING_WIN
#elif __APPLE__
  #include "TargetConditionals.h"
  #if TARGET_IPHONE_SIMULATOR
    #define SPIRE_USING_IOS
    #define SPIRE_OPENGL_ES_2
  #elif TARGET_OS_IPHONE
    #define SPIRE_USING_IOS
    #define SPIRE_OPENGL_ES_2
  #elif TARGET_OS_MAC
    #define SPIRE_USING_OSX
  #else
    #error Unsupported mac platform.
  #endif
#elif __ANDROID__
  #define SPIRE_USING_ANDROID
  #define SPIRE_OPENGL_ES_2
#elif __linux
  #define SPIRE_USING_LINUX
#elif __unix // all unices not caught above
  #error General unix not supported - try defining SPIRE_USING_LINUX and disabling this error.
#elif __posix
  #error General posix not supported - try defining SPIRE_USING_LINUX and disabling this error.
#else
  #error Unknown unsupported platform.
#endif

// OpenGL headers
#if defined(SPIRE_USING_OSX)
  #if defined(SPIRE_USING_WIN) || defined(SPIRE_USING_LINUX) || defined(SPIRE_USING_IOS) || defined(SPIRE_USING_ANDROID)
    #error Multiple platforms defined.
  #endif

  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
  #include <OpenGL/glu.h>
  #if defined(USE_CORE_PROFILE_3) || defined(USE_CORE_PROFILE_4)
    // Currently mac places gl4 specific definitions in the gl3 header. Change
    // when they update this.
    #include <OpenGL/gl3.h> 
  #endif
#elif defined(SPIRE_USING_WIN)

  #if defined(SPIRE_USING_OSX) || defined(SPIRE_USING_LINUX) || defined(SPIRE_USING_IOS) || defined(SPIRE_USING_ANDROID)
    #error Multiple platforms defined.
  #endif

  #define NOMINMAX
  #include <Windows.h>
  #include <glew/include/GL/glew.h>
  #include <GL/gl.h>
#elif defined(SPIRE_USING_LINUX)

  #if defined(SPIRE_USING_OSX) || defined(SPIRE_USING_WIN) || defined(SPIRE_USING_IOS) || defined(SPIRE_USING_ANDROID)
    #error Multiple platforms defined.
  #endif

  #define GL_GLEXT_PROTOTYPES
  #include <GL/gl.h>
  #include <GL/glext.h>
  #include <GL/glu.h>
  #include <GL/glx.h>
#elif defined(SPIRE_USING_IOS)

  #if defined(SPIRE_USING_OSX) || defined(SPIRE_USING_WIN) || defined(SPIRE_USING_LINUX) || defined(SPIRE_USING_ANDROID)
    #error Multiple platforms defined.
  #endif

  #import <OpenGLES/ES2/gl.h>
  #import <OpenGLES/ES2/glext.h>
#elif defined(SPIRE_USING_ANDROID)

  #if defined(SPIRE_USING_OSX) || defined(SPIRE_USING_WIN) || defined(SPIRE_USING_LINUX) || defined(SPIRE_USING_IOS)
    #error Multiple platforms defined.
  #endif

  #include <GLES2/gl2.h>
  #include <GLES2/gl2ext.h>
#else
  /// \todo Look into emscriptem.
  #error OpenGL headers not defined for this platform.
#endif

// Utility definitions for non-ES OpenGL implementations.
#ifndef SPIRE_OPENGL_ES_2
  #define GL_HALF_FLOAT_OES GL_FLOAT
#endif




#include <iostream>

#define MAX_GL_ERROR_COUNT 10 
#ifdef SPIRE_DEBUG
# define GL(stmt)                                                      \
  do {                                                                 \
    GLenum glerr;                                                      \
    unsigned int iCounter = 0;                                         \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      std::cerr << "GL error calling" << #stmt << " before line " << __LINE__ << " (" << __FILE__ << "): " << gluErrorString(glerr) << " (" << static_cast<unsigned>(glerr) << ")" << std::endl; \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
    stmt;                                                              \
    iCounter = 0;                                                      \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      std::cerr << "'" << #stmt << "' on line " << __LINE__ << " (" << __FILE__ << ") caused GL error: " << gluErrorString(glerr) << " (" << static_cast<unsigned>(glerr) << ")" << std::endl; \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
  } while(0)

# define GL_CHECK()                                                       \
  do {                                                                    \
    GLenum glerr;                                                         \
    unsigned int iCounter = 0;                                            \
    while((glerr = glGetError()) != GL_NO_ERROR) {                        \
      std::cerr << "GL error before line " << __LINE__ << "("   \
                << __FILE__ << "): " << glerr << " ("                  \
                << gluErrorString(glerr) << ")" << std::endl;          \
      iCounter++;                                                         \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                           \
    }                                                                     \
  } while(0)
#else
# define GL(stmt) do { stmt; } while(0)
# define GL_CHECK() 
#endif






#endif
