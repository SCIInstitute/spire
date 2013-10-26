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

#include "IBOObject.h"

namespace CPM_SPIRE_NS {

IBOObject::IBOObject(std::shared_ptr<std::vector<uint8_t>> iboData,
                     Interface::IBO_TYPE type)
{
  buildIBOObject(&(*iboData)[0], iboData->size(), type);
}

IBOObject::IBOObject(const uint8_t* iboData, size_t iboDataSize,
                     Interface::IBO_TYPE type)
{
  buildIBOObject(iboData, iboDataSize, type);
}

IBOObject::~IBOObject()
{
  GL(glDeleteBuffers(1, &mGLIndex));
}


void IBOObject::buildIBOObject(const uint8_t* iboData, size_t iboDataSize,
                               Interface::IBO_TYPE type)
{
  GL(glGenBuffers(1, &mGLIndex));
  GL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mGLIndex));
  GL(glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(iboDataSize),
                  iboData, GL_STATIC_DRAW));

  // Calculate number of elements based on the IBO type.
  switch (type)
  {
    case Interface::IBO_8BIT:
      mNumElements = static_cast<GLuint>(iboDataSize / sizeof(uint8_t));
      mType = GL_UNSIGNED_BYTE;
      break;

    case Interface::IBO_16BIT:
      mNumElements = static_cast<GLuint>(iboDataSize / sizeof(uint16_t));
      mType = GL_UNSIGNED_SHORT;
      break;

    case Interface::IBO_32BIT:
      mNumElements = static_cast<GLuint>(iboDataSize / sizeof(uint32_t));
      mType = GL_UNSIGNED_INT;
      break;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunreachable-code"
    default:
      throw std::invalid_argument("IBO type expected to be of type Interface::IBO_TYPE.");
      break;
#pragma clang diagnostic pop

  }
}

} // namespace CPM_SPIRE_NS

