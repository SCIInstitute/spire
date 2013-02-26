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

#include "gtest/gtest.h"
#include "Spire/3rdParty/GLEW/include/GL/glew.h"

#include "BatchContext.h"
#include "Spire/Tests/GlobalTestEnvironment.h"

#define cimg_display 0
#include "3rdParty/CImg/CImg.h"

class TestEnvironment : public Spire::GlobalTestEnvironmentInterface
{
public:
  TestEnvironment(uint32_t width, uint32_t height, 
                  int32_t colorBits, int32_t depthBits, int32_t stencilBits,
                  bool doubleBuffer, bool visible)
  {
    mContext = createContext(width, height, colorBits, depthBits, stencilBits,
                             doubleBuffer, visible);
  }


  /// Creates a context if one hasn't already been created and returns it.
  /// Otherwise, it returns the currently active context.
  std::shared_ptr<Spire::Context> getContext()
  {
    return std::dynamic_pointer_cast<Spire::Context>(mContext);
  }

  /// Overrides from gtest
  /// @{
  void SetUp() override     {}
  void TearDown() override  {}
  /// @}

private:
  static std::shared_ptr<Spire::BatchContext> createContext(
      uint32_t width, uint32_t height,
      int32_t colorBits, int32_t depthBits, int32_t stencilBits,
      bool doubleBuffer, bool visible)
  {
    std::shared_ptr<Spire::BatchContext> ctx(
        Spire::BatchContext::Create(width,height,
                                    colorBits,depthBits,stencilBits, 
                                    doubleBuffer,visible));
    if (ctx->isValid() == false)
      throw std::runtime_error("Invalid context generated.");
    ctx->makeCurrent();

    return ctx;
  }

  std::shared_ptr<Spire::BatchContext> mContext;
};


int main(int argc, char** argv)
{
  // Create the global testing environment.
  ::testing::Environment* const testEnv = 
      ::testing::AddGlobalTestEnvironment(
          new TestEnvironment(640, 480, 32, 24, 8, true, false));

  // Run the tests.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

