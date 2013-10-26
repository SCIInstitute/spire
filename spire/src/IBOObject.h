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

#ifndef SPIRE_STUPIPE_STUIBOOBJECT_H
#define SPIRE_STUPIPE_STUIBOOBJECT_H

#include <vector>
#include <memory>

#include "Common.h"
#include "../Interface.h"

namespace CPM_SPIRE_NS {

/// Object that encapsulates an OpenGL index buffer. The buffer will be
/// automatically deleted by IBOObject's destructor.
class IBOObject
{
public:
  // This constructor delegates to the raw version below.
  IBOObject(std::shared_ptr<std::vector<uint8_t>> iboData,
            Interface::IBO_TYPE type);

  IBOObject(const uint8_t* iboData, size_t iboDataSize, Interface::IBO_TYPE type);
  ~IBOObject();

  GLuint getGLIndex() const               {return mGLIndex;}
  GLuint getNumElements() const           {return mNumElements;}
  GLenum getType() const                  {return mType;}

private:

  void buildIBOObject(const uint8_t* iboData, size_t iboDataSize,
                      Interface::IBO_TYPE type);

  GLuint                    mGLIndex;    ///< Corresponds to the map index but obtained from OpenGL.
  GLuint                    mNumElements;///< Number of elements in the IBO.
  GLenum                    mType;       ///< Type of index buffer.
};

} // namespace CPM_SPIRE_NS

#endif 
