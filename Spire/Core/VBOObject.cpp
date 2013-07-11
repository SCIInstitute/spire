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

#include "VBOObject.h"

namespace Spire {

//------------------------------------------------------------------------------
VBOObject::VBOObject(
    std::shared_ptr<std::vector<uint8_t>> vboData,
    const std::vector<std::string>& attributes,
    const ShaderAttributeMan& man)
    : mAttributeCollection(man)
{
  GL(glGenBuffers(1, &mGLIndex));
  GL(glBindBuffer(GL_ARRAY_BUFFER, mGLIndex));
  GL(glBufferData(GL_ARRAY_BUFFER, vboData->size(), &(*vboData)[0], GL_STATIC_DRAW));

  for (auto it = attributes.begin(); it != attributes.end(); ++it)
  {
    mAttributeCollection.addAttribute(*it, false);
  }
}

//------------------------------------------------------------------------------
VBOObject::~VBOObject()
{
  GL(glDeleteBuffers(1, &mGLIndex));
}


} // end of namespace Spire
