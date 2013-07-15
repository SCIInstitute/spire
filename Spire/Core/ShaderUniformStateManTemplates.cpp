/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2013 Scientific Computing and Imaging Institute,
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
/// \date   February 2013

#include "Common.h"
#include "ShaderUniformStateManTemplates.h"

namespace Spire {

//------------------------------------------------------------------------------
// GL uniform implementation.
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
void AbstractUniformStateItem::uniform1f(int location, float v0)
{
  GL(glUniform1f(static_cast<GLint>(location), v0));
}

//------------------------------------------------------------------------------
void AbstractUniformStateItem::uniform2f(int location, float v0, float v1)
              
{
  // Technically, vn should be cast to GLfloat...
  GL(glUniform2f(static_cast<GLint>(location), v0, v1));
}

//------------------------------------------------------------------------------
void AbstractUniformStateItem::uniform3f(int location, float v0, float v1,
                                         float v2)
{
  // Technically, vn should be cast to GLfloat...
  GL(glUniform3f(static_cast<GLint>(location), v0, v1, v2));
}

//------------------------------------------------------------------------------
void AbstractUniformStateItem::uniform3fv(int location, size_t count,
                                          const float* value)
{
  GL(glUniform4fv(static_cast<GLint>(location), static_cast<GLsizei>(count),
                  static_cast<const GLfloat*>(value)));
}

//------------------------------------------------------------------------------
void AbstractUniformStateItem::uniform4f(int location, float v0, float v1,
                                         float v2, float v3)
{
  GL(glUniform4f(static_cast<GLint>(location), v0, v1, v2, v3));
}

//------------------------------------------------------------------------------
void AbstractUniformStateItem::uniformMatrix4fv(int location, size_t count, 
                                                bool transpose, const float* value)
{
  GL(glUniformMatrix4fv(static_cast<GLint>(location), static_cast<GLsizei>(count), transpose,
                        static_cast<const GLfloat*>(value)));
}

} // end of namespace Spire
