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

#include "gtest/gtest.h"

#include "Common.h"
#include "Exceptions.h"
#include "StuPipe/StuInterface.h"
#include "StuPipe/StuObject.h"

#include "GlobalTestEnvironment.h"
#include "CommonTestFixtures.h"

using namespace Spire;

namespace {

//------------------------------------------------------------------------------
TEST_F(StuPipeTestFixture, TestPublicInterface)
{
  // This test is contrived and won't yield that much knowledge if you are 
  // attempting to learn the system.

  // REMEMBER:  We will always run the tests synchronously! So we will be able
  //            to catch errors immediately.

  std::string obj1 = "obj1";
  std::string obj2 = "obj2";
  std::string obj3 = "obj3";

  // We have a fresh instance of spire with a StuPipe bound.
  mStuInterface->addObject(obj1);
  EXPECT_EQ(0, mStuInterface->ntsGetObjectWithName(obj1)->getRenderOrder());
  EXPECT_THROW(mStuInterface->addObject(obj1), Duplicate);
  EXPECT_EQ(1, mStuInterface->ntsGetNumObjects());

  // Add a new obj2.
  mStuInterface->addObject(obj2);
  EXPECT_EQ(1, mStuInterface->ntsGetObjectWithName(obj2)->getRenderOrder());
  EXPECT_THROW(mStuInterface->addObject(obj1), Duplicate);
  EXPECT_THROW(mStuInterface->addObject(obj2), Duplicate);
  EXPECT_EQ(2, mStuInterface->ntsGetNumObjects());

  EXPECT_TRUE(mStuInterface->ntsHasRenderingOrder({obj1, obj2}));

  // Remove and re-add object 1.
  mStuInterface->removeObject(obj1);
  EXPECT_EQ(1, mStuInterface->ntsGetNumObjects());
  mStuInterface->addObject(obj1);
  EXPECT_EQ(2, mStuInterface->ntsGetObjectWithName(obj1)->getRenderOrder());
  EXPECT_EQ(2, mStuInterface->ntsGetNumObjects());

  EXPECT_TRUE(mStuInterface->ntsHasRenderingOrder({obj2, obj1}));

  // Add a new obj3.
  mStuInterface->addObject(obj3);
  EXPECT_EQ(3, mStuInterface->ntsGetObjectWithName(obj3)->getRenderOrder());
  EXPECT_THROW(mStuInterface->addObject(obj1), Duplicate);
  EXPECT_THROW(mStuInterface->addObject(obj2), Duplicate);
  EXPECT_THROW(mStuInterface->addObject(obj3), Duplicate);
  EXPECT_EQ(3, mStuInterface->ntsGetNumObjects());

  EXPECT_TRUE(mStuInterface->ntsHasRenderingOrder({obj2, obj1, obj3}));

  // Test render re-ordering.
  mStuInterface->assignRenderOrder(obj1, 5);
  mStuInterface->assignRenderOrder(obj2, 1);
  mStuInterface->assignRenderOrder(obj3, 2);
  EXPECT_EQ(5, mStuInterface->ntsGetObjectWithName(obj1)->getRenderOrder());
  EXPECT_EQ(1, mStuInterface->ntsGetObjectWithName(obj2)->getRenderOrder());
  EXPECT_EQ(2, mStuInterface->ntsGetObjectWithName(obj3)->getRenderOrder());
  EXPECT_TRUE(mStuInterface->ntsHasRenderingOrder({obj2, obj3, obj1}));

  mStuInterface->assignRenderOrder(obj3, 5);
  mStuInterface->assignRenderOrder(obj2, 2);
  mStuInterface->assignRenderOrder(obj1, 1);
  EXPECT_EQ(5, mStuInterface->ntsGetObjectWithName(obj3)->getRenderOrder());
  EXPECT_EQ(2, mStuInterface->ntsGetObjectWithName(obj2)->getRenderOrder());
  EXPECT_EQ(1, mStuInterface->ntsGetObjectWithName(obj1)->getRenderOrder());
  EXPECT_TRUE(mStuInterface->ntsHasRenderingOrder({obj1, obj2, obj3}));

  mStuInterface->assignRenderOrder(obj1, 3);
  mStuInterface->assignRenderOrder(obj2, 5);
  mStuInterface->assignRenderOrder(obj3, 1);
  EXPECT_EQ(3, mStuInterface->ntsGetObjectWithName(obj1)->getRenderOrder());
  EXPECT_EQ(5, mStuInterface->ntsGetObjectWithName(obj2)->getRenderOrder());
  EXPECT_EQ(1, mStuInterface->ntsGetObjectWithName(obj3)->getRenderOrder());
  EXPECT_TRUE(mStuInterface->ntsHasRenderingOrder({obj3, obj1, obj2}));
}

//------------------------------------------------------------------------------
TEST_F(StuPipeTestFixture, TestTriangle)
{
  // Test the rendering of a triangle with StuPipe.

  // Call Interface's doFrame manually. Then, since we are single threaded,
  // use the OpenGL context to extract a frame from the GPU and compare it with
  // golden images generated prior. If no golden image exists for this run,
  // then manually add it to the golden image comparison storage area...

  // First things first: just get the rendered image onto the filesystem...
  
  std::vector<float> vboData = 
  {
    -1.0f,  1.0f,  0.0f,
    1.0f,  1.0f,  0.0f,
    -1.0f, -1.0f,  0.0f,
    1.0f, -1.0f,  0.0f
  };
  std::vector<std::string> attribNames = {"aPos"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };
  StuInterface::IBO_TYPE iboType = StuInterface::IBO_16BIT;

  // This is pretty contorted interface due to the marshalling between
  // std::vector<float> and std::vector<uint8_t>. In practice, you would want
  // to calculate the size of your VBO and using one std::vector<uint8_t> and
  // reserve the necessary space in it. Then cast it's contents to floats or
  // uint16_t as necessary (attributes can have a wide array of types, including
  // half floats).
  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  std::copy(rawBegin, rawBegin + rawSize, rawVBO->begin());

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Test attempting to add VBO and IBO before creating the object...
  std::string obj1 = "obj1";
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";

  EXPECT_THROW(mStuInterface->addVBOToObject(obj1, vbo1, rawVBO, attribNames), std::out_of_range);
  EXPECT_THROW(mStuInterface->addIBOToObject(obj1, ibo1, rawIBO, iboType), std::out_of_range);

  mStuInterface->addObject(obj1);
  mStuInterface->addVBOToObject(obj1, vbo1, rawVBO, attribNames);
  mStuInterface->addIBOToObject(obj1, ibo1, rawIBO, iboType);
  EXPECT_THROW(mStuInterface->addVBOToObject(obj1, vbo1, rawVBO, attribNames), Duplicate);
  EXPECT_THROW(mStuInterface->addIBOToObject(obj1, ibo1, rawIBO, iboType), Duplicate);
  
  // Add and compile persistent shaders.
  mStuInterface->addPersistentShader(
      "UniformColor", 
      { {"UniformColor.vs", StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fs", StuInterface::FRAGMENT_SHADER},
      });

  // Test various cases of shader failure after adding a prior shader.
  EXPECT_THROW(mStuInterface->addPersistentShader(
      "UniformColor", 
      { {"UniformColor.vs", StuInterface::FRAGMENT_SHADER}, 
        {"UniformColor.fs", StuInterface::VERTEX_SHADER},
      }), std::invalid_argument);

  EXPECT_THROW(mStuInterface->addPersistentShader(
      "UniformColor", 
      { {"UniformColor2.vs", StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fs", StuInterface::FRAGMENT_SHADER},
      }), std::invalid_argument);

  EXPECT_THROW(mStuInterface->addPersistentShader(
      "UniformColor", 
      { {"UniformColor.vs", StuInterface::VERTEX_SHADER}, 
        {"UniformColor2.fs", StuInterface::FRAGMENT_SHADER},
      }), std::invalid_argument);

  // This final exception is throw directly from the addPersistentShader
  // function. The 3 prior exception were all thrown from the ShaderProgramMan.
  EXPECT_THROW(mStuInterface->addPersistentShader(
      "UniformColor", 
      { {"UniformColor.vs", StuInterface::VERTEX_SHADER}, 
        {"UniformColor.fs", StuInterface::FRAGMENT_SHADER},
      }), Duplicate);

  /// \todo Test attributes in shader against attributes specified by vbo.
  
}

//------------------------------------------------------------------------------
TEST_F(StuPipeTestFixture, TestCube)
{
  // Test the rendering of a cube with the StuPipe
}

}

