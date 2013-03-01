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
#include "Spire/Common.h"
#include "Spire/Core/Log.h"

// Defining cimg_display to 0 ensures CImg doesn't try to include OS specific
// windowing header files.
#define cimg_display 0
#include "3rdParty/CImg/CImg.h"
using namespace cimg_library;

// Lots of yucky code in the test-environment.
class TestEnvironment : public Spire::GlobalTestEnvironment
{
public:
  TestEnvironment(uint32_t width, uint32_t height, 
                  int32_t colorBits, int32_t depthBits, int32_t stencilBits,
                  bool doubleBuffer, bool visible) :
      mWidth(width),
      mHeight(height)
  {
    mContext = createContext(width, height, colorBits, depthBits, stencilBits,
                             doubleBuffer, visible);

    // Pre-allocate an image of appropriate size.
    try
    {
      mRawImage.resize(width*height*4);  // 4 is the bit depth - RGBA.
    }
    catch (...)
    {
      mRawImage.clear();
    }
    if ( mRawImage.empty() )
      throw std::runtime_error("Unable to allocate space for image.");
  }


  /// Creates a context if one hasn't already been created and returns it.
  /// Otherwise, it returns the currently active context.
  std::shared_ptr<Spire::Context> getContext() const override
  {
    return std::dynamic_pointer_cast<Spire::Context>(mContext);
  }

  void writeFBO(const std::string& file) override
  {
    // This function should be called from test code. So we are safe writing
    // test assertions in this code.
    GLint viewport[4];
    GL(glGetIntegerv(GL_VIEWPORT, viewport));
    ASSERT_EQ(0, viewport[0]);
    ASSERT_EQ(0, viewport[1]);
    ASSERT_EQ(mWidth, viewport[2]);
    ASSERT_EQ(mHeight, viewport[3]);

    GL(glPixelStorei(GL_PACK_ALIGNMENT, 1));
    GL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL(glReadBuffer(GL_BACK));
    mRawImage[0] = 255;
    mRawImage[1] = 255;
    mRawImage[2] = 255;
    mRawImage[3] = 177;
    GL_CHECK();
    GL(glReadPixels(0,0,viewport[2],viewport[3],GL_RGBA,GL_UNSIGNED_BYTE,&mRawImage[0]));
    GL_CHECK();
#ifdef SPIRE_DEBUG
    Spire::Log::debug() << "Test" << std::endl;
#endif
    std::cerr << "Image Data: " << static_cast<int>(mRawImage[0]) 
                                << static_cast<int>(mRawImage[1]) 
                                << static_cast<int>(mRawImage[2]) 
                                << static_cast<int>(mRawImage[3]) << std::endl;

    //CImg<uint8_t> img(viewport[2], viewport[3], 1, 4, 0);
    // We are using shared memory for the CImg class so it doesn't have to allocate its own.
    // It would be better to std::move vector into CImg though.
    // \todo image flipped?
    CImg<uint8_t> img(reinterpret_cast<uint8_t*>(&mRawImage[0]), viewport[2],
                      viewport[3], 1, 4, true);
    img.save(file.c_str());
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

    // Setup the viewport correctly.
    GL(glViewport(0, 0, width, height));

    return ctx;
  }

  std::shared_ptr<Spire::BatchContext> mContext;

  /// Pre-allocated rawImage of what we will pull from OpenGL.
  std::vector<uint8_t> mRawImage;
  uint32_t mWidth;
  uint32_t mHeight;
};


int main(int argc, char** argv)
{
  // Create the global testing environment.
  ::testing::Environment* const testEnv = 
      ::testing::AddGlobalTestEnvironment(
          new TestEnvironment(600, 600, 32, 24, 8, true, false));

  // Run the tests.
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

