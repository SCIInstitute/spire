/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
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

#include <cstdlib>
#include <iostream>

#include "GL/glew.h"

#include "BatchContext.h"

std::shared_ptr<Spire::BatchContext> createContext(uint32_t width, uint32_t height,
                                                   int32_t color_bits,
                                                   int32_t depth_bits,
                                                   int32_t stencil_bits,
                                                   bool double_buffer, bool visible)
{
  std::shared_ptr<Spire::BatchContext> ctx(
      Spire::BatchContext::Create(width,height, color_bits,depth_bits,stencil_bits, 
                           double_buffer,visible));
  ctx->isValid();
  ctx->makeCurrent();

  return ctx;
}

int main(int argc, const char* argv[])
{
  // Read Lua filename from the first program argument
  std::string filename;
}

