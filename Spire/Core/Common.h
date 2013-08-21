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

/// \author James Hughes
/// \date   December 2012

#ifndef SPIRE_COMMON_H
#define SPIRE_COMMON_H

#include <cstddef>

#include "../Interface.h"

#if defined(USE_CORE_PROFILE_3) && defined(USE_CORE_PROFILE_4)
  #error Both USE_CORE_PROFILE_3 and USE_CORE_PROFILE_4 are defined. Ensure that only one is defined.
#endif

// OpenGL headers
#ifdef SPIRE_USING_OSX
  #include <OpenGL/gl.h>
  #include <OpenGL/glext.h>
  #include <OpenGL/glu.h>
  #if defined(USE_CORE_PROFILE_3) || defined(USE_CORE_PROFILE_4)
    // Currently mac places gl4 specific definitions in the gl3 header. Change
    // when they update this.
    #include <OpenGL/gl3.h> 
  #endif
#elif SPIRE_USING_WIN
  #define NOMINMAX
  #include <Windows.h>
  #include <GL/glew.h>
  #include <GL/gl.h>
  #include <GL/glext.h>
#elif SPIRE_USING_LINUX
  #include <GL/gl.h>
  #include <GL/glext.h>
  #include <GL/glx.h>
#elif SPIRE_USING_IOS
  #import <OpenGLES/ES2/gl.h>
  #import <OpenGLES/ES2/glext.h>
#elif SPIRE_USING_ANDROID
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

// Any ubiquitous header files.
#include "Core/Math.h"
#include "Core/Log.h"

#define MAX_GL_ERROR_COUNT 10 

#ifdef SPIRE_DEBUG
# define GL(stmt)                                                      \
  do {                                                                 \
    GLenum glerr;                                                      \
    unsigned int iCounter = 0;                                         \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      Spire::Log::error() << "GL error calling" << #stmt << " before line " << __LINE__ << " (" << __FILE__ << "): " << gluErrorString(glerr) << " (" << static_cast<unsigned>(glerr) << ")" << std::endl; \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
    stmt;                                                              \
    iCounter = 0;                                                      \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      Spire::Log::error() << "'" << #stmt << "' on line " << __LINE__ << " (" << __FILE__ << ") caused GL error: " << gluErrorString(glerr) << " (" << static_cast<unsigned>(glerr) << ")" << std::endl; \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
  } while(0)

# define GL_CHECK()                                                       \
  do {                                                                    \
    GLenum glerr;                                                         \
    unsigned int iCounter = 0;                                            \
    while((glerr = glGetError()) != GL_NO_ERROR) {                        \
      Spire::Log::error() << "GL error before line " << __LINE__ << "("   \
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

namespace Spire
{

} // namespace Spire

#endif
