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

#ifndef SPIRE_CORE_HACKEDUCRENDERER_H
#define SPIRE_CORE_HACKEDUCRENDERER_H

#include "Core/Hub.h"

namespace Spire {

class ShaderProgramAsset;

/// A hacked uniform color renderer for demonstration purposes only.
class HackedUCRenderer
{
public:
  HackedUCRenderer(Hub& hub);
  virtual ~HackedUCRenderer();
  
  void doFrame();

  void setCommonVBO(uint8_t* vbo, size_t vboSize);
  void setEdgeData(uint8_t* ibo, size_t iboSize);
  void setEdgeColor(const V4& color);
  void setFaceData(uint8_t* ibo, size_t iboSize);
  void setFaceColor(const V4& color);
  void setUseZTest(bool ztest)  {mUseZTest = ztest;}

  bool getUseZTest()            {return mUseZTest;}

private:

  GLuint  mCommonVBO;

  GLuint  mFaceIBO;
  V4      mFaceColor;
  size_t  mNumFaceElements;

  GLuint  mEdgeIBO;
  V4      mEdgeColor;
  size_t  mNumEdgeElements;

  bool    mUseZTest;

  Hub&    mHub;

  std::shared_ptr<ShaderProgramAsset>  mShader;
};

} // namespace Spire

#endif 
