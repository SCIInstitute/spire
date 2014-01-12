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

#include <batch-testing/GlobalGTestEnv.hpp>
#include <batch-testing/SpireTestFixture.hpp>
#include "namespaces.h"

#include "spire/src/Common.h"
#include "spire/src/Exceptions.h"
#include "spire/src/SpireObject.h"
#include "spire/src/FileUtil.h"

#include "TestCommonUniforms.h"
#include "TestCommonAttributes.h"
#include "TestCamera.h"

using namespace spire;
using namespace CPM_BATCH_TESTING_NS;

namespace {

//------------------------------------------------------------------------------
TEST_F(SpireTestFixture, TestConcurrentQuad)
{
  std::unique_ptr<TestCamera> myCamera = std::unique_ptr<TestCamera>(new TestCamera);
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
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, reinterpret_cast<uint8_t*>(&vboData[0]), vboData.size() * sizeof(float), attribNames);
  mSpire->addIBO(ibo1, reinterpret_cast<uint8_t*>(&iboData[0]), iboData.size() * sizeof(uint16_t), iboType);

  // Attempt to add duplicate VBOs and IBOs
  EXPECT_THROW(mSpire->addVBO(vbo1, reinterpret_cast<uint8_t*>(&vboData[0]), vboData.size(), attribNames), Duplicate);
  EXPECT_THROW(mSpire->addIBO(ibo1, reinterpret_cast<uint8_t*>(&iboData[0]), iboData.size(), iboType), Duplicate);

  std::string obj1 = "obj1";
  mSpire->addObject(obj1);
  
  std::string shader1 = "UniformColor";
  // Add and compile persistent shaders (if not already present).
  // You will only run into the 'Duplicate' exception if the persistent shader
  // is already in the persistent shader list.
  mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      });

  // Build a pass to use. Strictly don't need to do this, as we could just use
  // the default pass. But we are here to test!
  std::string pass1 = "pass1";

  /// \todo We need to test front / back geom pass, not this add pass to
  ///       object business.
  // Now add the object pass. This automatically adds the object to the pass for
  // us. But the ordering within the pass is still arbitrary.
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(ibo1);
  mSpire->removeVBO(vbo1);
  EXPECT_THROW(mSpire->removeIBO(ibo1), std::out_of_range);
  EXPECT_THROW(mSpire->removeVBO(vbo1), std::out_of_range);

  // Test global uniforms -- test run-time type validation.
  // Setup camera so that it can be passed to the Uniform Color shader.
  // Camera has been setup in the test fixture.
  mSpire->addGlobalUniform("uProjIVObject", myCamera->getWorldToProjection());
  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f), pass1);

  // Perform the rendering of JUST the object that we created.
  // Need to test adding various different objects to the scene and attempting
  // to render them.
  beginFrame();   // From SpireTestFixture.
  mSpire->renderObject(obj1, pass1);  

  compareFBOWithExistingFile(
      "concurrentQuad.png",
      TEST_IMAGE_OUTPUT_DIR,
      TEST_IMAGE_COMPARE_DIR,
      TEST_PERCEPTUAL_COMPARE_BINARY,
      50);
}


}


