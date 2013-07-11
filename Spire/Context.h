/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
   University of Utah.

   License for the specific language governing rights and limitations under
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
/// \date   September 2012

#ifndef SPIRE_CONTEXT_H
#define SPIRE_CONTEXT_H

#include "Core/WinDLLExport.h"

namespace Spire {

/// Generalized OpenGL context.
/// With the existence of wrappers like http://code.google.com/p/angleproject/,
/// writing OpenGL ES 2.0 compliant code means we can run using DirectX on 
/// windows (better driver support).
/// All functions contained herein must be thread safe as they will be executed
/// from the Spire's thread.
class WIN_DLL Context
{
public:
  Context()             {}
  virtual ~Context()    {}

  //============================================================================
  // Mandatory OpenGL-context related functions
  //============================================================================

  /// Make the context current on the active thread.
  virtual void makeCurrent()    = 0;

  /// Swap the front and back buffers.
  virtual void swapBuffers()    = 0;

private:
};

} // namespace spire

#endif // SPIRE_CONTEXT_H
