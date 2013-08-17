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
/// \date   August 2013

#ifndef SRUTIL_H
#define SRUTIL_H

#include <string>
#include <vector>

namespace Spire {
namespace SCIRun {

// Misc SCIRun utilities.

/// Normal rendering. Expects position and normals to be floats.
/// \param  vboData     The raw VBO that gets sent to spire.
/// \param  stride      Stride between vertices.
/// \param  posOffset   Offset to position.
/// \param  normOffset  Offset to the normal.
/// \param  out_vboData Output VBO data. Format: Position, normal. Where there
///                     3 elements inside the position and normal and each 
///                     element is a float.
/// \param  out_iboData Output IBO data. Format: uint16_t, GL_LINE.
/// \return The number of elements (GL_LINE) in the ibo data.
size_t buildNormalRenderingForVBO(std::shared_ptr<std::vector<uint8_t>> vboData,
                                  size_t stride,
                                  std::vector<uint8_t>& out_vboData,
                                  std::vector<uint8_t>& out_iboData,
                                  size_t posOffset = 0,
                                  size_t normOffset = sizeof(float) * 3);

} // namespace SCIRun
} // namespace Spire

#endif 
