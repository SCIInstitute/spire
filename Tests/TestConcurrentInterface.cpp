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

#include "GlobalTestEnvironment.h"

#include "Spire/Core/Common.h"
#include "Spire/Core/Exceptions.h"
#include "Spire/Core/SpireObject.h"
#include "Spire/Core/FileUtil.h"
#include "Spire/Core/LambdaInterface.h"
#include "Spire/Core/ObjectLambda.h"

#include "TestCommonUniforms.h"
#include "TestCommonAttributes.h"
#include "CommonTestFixtures.h"

using namespace Spire;

namespace {

// Simple function to handle object transformations so that the GPU does not
// need to do the same calculation for each vertex.
static void lambdaUniformObjTrafs(ObjectLambdaInterface& iface, 
                                  std::list<Interface::UnsatisfiedUniform>& unsatisfiedUniforms)
{
  // Cache object to world transform.
  M44 objToWorld = iface.getObjectMetadata<M44>(
      std::get<0>(TestCommonAttributes::getObjectToWorldTrafo()));

  std::string objectTrafoName = std::get<0>(TestCommonUniforms::getObject());
  std::string objectToViewName = std::get<0>(TestCommonUniforms::getObjectToView());
  std::string objectToCamProjName = std::get<0>(TestCommonUniforms::getObjectToCameraToProjection());

  // Loop through the unsatisfied uniforms and see if we can provide any.
  for (auto it = unsatisfiedUniforms.begin(); it != unsatisfiedUniforms.end(); /*nothing*/ )
  {
    if (it->uniformName == objectTrafoName)
    {
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToViewName)
    {
      // Grab the inverse view transform.
      M44 inverseView = glm::affineInverse(
          iface.getGlobalUniform<M44>(std::get<0>(TestCommonUniforms::getCameraToWorld())));
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseView * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToCamProjName)
    {
      M44 inverseViewProjection = iface.getGlobalUniform<M44>(
          std::get<0>(TestCommonUniforms::getToCameraToProjection()));
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseViewProjection * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else
    {
      ++it;
    }
  }
}


//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestConcurrentQuad)
{
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
  mSpire->addPassToBack(pass1);

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
  mSpire->addGlobalUniform("uProjIVObject", mCamera->getWorldToProjection());
  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f), pass1);

  // Perform the rendering of JUST the object that we created.
  // Need to test adding various different objects to the scene and attempting
  // to render them.
  mSpire->beginFrame(true);
  mSpire->renderObject(obj1, nullptr, pass1);  
  mSpire->endFrame();

  // Write the resultant png to a temporary directory and compare against
  // the golden image results.
#ifdef TEST_OUTPUT_IMAGES
  std::string thisImage       = "concurrentQuad.png";
  std::string comparisonImage = "stuTriangle.png";

  std::string targetImage = TEST_IMAGE_OUTPUT_DIR;
  targetImage += "/" + thisImage;
  Spire::GlobalTestEnvironment::instance()->writeFBO(targetImage);

  EXPECT_TRUE(Spire::fileExists(targetImage)) << "Failed to write output image! " << targetImage;

#ifdef TEST_PERCEPTUAL_COMPARE
  // Perform the perceptual comparison using the given regression directory.
  std::string compImage = TEST_IMAGE_COMPARE_DIR;
  compImage += "/" + comparisonImage;

  ASSERT_TRUE(Spire::fileExists(compImage)) << "Failed to find comparison image! " << compImage;
  // Test using perceptula comparison program that the user has provided
  // (hopefully).
  std::string command = TEST_PERCEPTUAL_COMPARE_BINARY;
  command += " -threshold 50 ";
  command += targetImage + " " + compImage;

  // Usually the return code of std::system is implementation specific. But the
  // majority of systems end up returning the exit code of the program.
  if (std::system(command.c_str()) != 0)
  {
    // The images are NOT the same. Alert the user.
    FAIL() << "Perceptual compare of " << thisImage << " failed.";
  }
#endif

#endif
}

//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestObjectsStructure)
{
  // Test various functions in Object and ObjectPass.
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
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]);
  rawVBO->assign(rawBegin, rawBegin + rawSize);

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(uint16_t) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]);
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, rawVBO, attribNames);
  mSpire->addIBO(ibo1, rawIBO, iboType);

  // Attempt to add duplicate VBOs and IBOs
  EXPECT_THROW(mSpire->addVBO(vbo1, rawVBO, attribNames), Duplicate);
  EXPECT_THROW(mSpire->addIBO(ibo1, rawIBO, iboType), Duplicate);

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

  // Build the default pass.
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP);

  // Add a uniform *before* we add the next pass to ensure it gets properly
  // propogated to the new pass.
  mSpire->addObjectGlobalUniform(obj1, "uProjIVObject", mCamera->getWorldToProjection());

  // Construct another good pass.
  std::string pass1 = "pass1";
  mSpire->addPassToFront(pass1);
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(ibo1);
  mSpire->removeVBO(vbo1);

  //----------------------------------------------------------------------------
  // Test Interface structures
  //----------------------------------------------------------------------------
  EXPECT_EQ(true, mSpire->ntsHasPass(pass1));
  EXPECT_EQ(true, mSpire->ntsHasPass(SPIRE_DEFAULT_PASS));
  EXPECT_EQ(false, mSpire->ntsHasPass("nonexistant"));

  EXPECT_EQ(true, mSpire->ntsIsObjectInPass(obj1, pass1));
  EXPECT_EQ(true, mSpire->ntsIsObjectInPass(obj1, SPIRE_DEFAULT_PASS));
  EXPECT_EQ(false, mSpire->ntsIsObjectInPass(obj1, "nonexistant"));
  EXPECT_EQ(false, mSpire->ntsIsObjectInPass("nonexistant", pass1));
  EXPECT_EQ(false, mSpire->ntsIsObjectInPass("nonexistant", SPIRE_DEFAULT_PASS));

  // Add pass uniforms for each pass.
  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));    // default pass
  mSpire->addObjectGlobalUniform(obj1, "uColor", V4(1.0f, 0.0f, 1.0f, 1.0f));  // pass1

  //----------------------------------------------------------------------------
  // Test SpireObject structures
  //----------------------------------------------------------------------------
  std::shared_ptr<const SpireObject> object1 = mSpire->ntsGetObjectWithName(obj1);
  std::shared_ptr<const ObjectPass> object1Pass1 = object1->getObjectPassParams(pass1);
  std::shared_ptr<const ObjectPass> object1PassDefault = object1->getObjectPassParams(SPIRE_DEFAULT_PASS);

  EXPECT_EQ(2, object1->getNumPasses());
  EXPECT_EQ(true,  object1->hasGlobalUniform("uColor"));
  EXPECT_EQ(true,  object1->hasGlobalUniform("uProjIVObject"));
  EXPECT_EQ(false, object1->hasGlobalUniform("nonexistant"));

  EXPECT_EQ(false, object1Pass1->hasPassSpecificUniform("uColor"));
  EXPECT_EQ(true,  object1Pass1->hasUniform("uColor"));
  EXPECT_EQ(false, object1Pass1->hasPassSpecificUniform("uProjIVObject"));
  EXPECT_EQ(true,  object1Pass1->hasUniform("uProjIVObject"));

  EXPECT_EQ(true,  object1PassDefault->hasPassSpecificUniform("uColor"));
  EXPECT_EQ(true,  object1PassDefault->hasUniform("uColor"));
  EXPECT_EQ(false, object1PassDefault->hasPassSpecificUniform("uProjIVObject"));
  EXPECT_EQ(true,  object1PassDefault->hasUniform("uProjIVObject"));

  // Test attributes
  M44 testUniform;
  testUniform[3] = V4(1.0f, 1.0f, 1.0f, 1.0f);
  mSpire->addObjectGlobalMetadata<M44>(obj1, "objectTransform", testUniform);
  mSpire->addObjectPassMetadata<M44>(obj1, "passTransform", testUniform, pass1);

  auto testMatrixEquality = [](const M44& a, const M44& b) {
    for (size_t c = 0; c < 4; c++)
    {
      for (size_t r = 0; r < 4; r++)
      {
        EXPECT_FLOAT_EQ(a[c][r], b[c][r]);
      }
    }
  };

  M44 retUnif;
  std::shared_ptr<const AbstractUniformStateItem> uniformItem;

  retUnif = object1->getObjectGlobalMetadata("objectTransform")->getData<M44>();
  testMatrixEquality(retUnif, testUniform);

  EXPECT_THROW(object1->getObjectGlobalMetadata("nonexistant"), std::runtime_error);

  uniformItem = object1->getObjectPassMetadata(pass1, "passTransform");
  retUnif = uniformItem->getData<M44>();
  testMatrixEquality(retUnif, testUniform);

  uniformItem = object1->getObjectPassMetadata(pass1, "nonexistant");
  EXPECT_EQ(nullptr, uniformItem);

  uniformItem = object1->getObjectPassMetadata(SPIRE_DEFAULT_PASS, "objectTransform");
  EXPECT_EQ(nullptr, uniformItem);

  // Perform the frame. If there are any missing shaders we'll know about it
  // here.
  mSpire->ntsDoFrame();
}

//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestRenderingWithSR5Object)
{
  // This test demonstrates a quick and dirty renderer using
  // attributes and lambdas. Spire knows nothing about the objects, but allows
  // sufficient flexibility that it is possible to do many things.

  // First things first: just get the rendered image onto the filesystem...
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  std::fstream sphereFile("Assets/UncappedCylinder.sp");
  Interface::loadProprietarySR5AssetFile(sphereFile, *rawVBO, *rawIBO);

  std::vector<std::string> attribNames = {"aPos", "aNormal"};
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // Add necessary VBO's and IBO's
  std::string vboName = "vbo1";
  std::string iboName = "ibo1";
  mSpire->addVBO(vboName, rawVBO, attribNames);
  mSpire->addIBO(iboName, rawIBO, iboType);

  // Build shaders
  std::string shaderName = "UniformColor";
  mSpire->addPersistentShader(
      shaderName, 
      { std::make_tuple("UniformColor.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("UniformColor.fsh", Interface::FRAGMENT_SHADER),
      });

  // Add object
  std::string objectName = "obj1";
  mSpire->addObject(objectName);
  mSpire->addPassToObject(objectName, shaderName, vboName, iboName, 
                          Interface::TRIANGLE_STRIP);
  mSpire->addLambdaObjectUniforms(objectName, lambdaUniformObjTrafs);
  
  // Object pass uniforms (can be set at a global level)
  mSpire->addObjectPassUniform(objectName, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f));    // default pass
  mSpire->addObjectGlobalUniform(objectName, "uProjIVObject", mCamera->getWorldToProjection());

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(iboName);
  mSpire->removeVBO(vboName);

  mSpire->ntsDoFrame();

  // Write the resultant png to a temporary directory and compare against
  // the golden image results.
  /// \todo Look into using boost filesystem (but it isn't header-only). 

#ifdef TEST_OUTPUT_IMAGES
  std::string imageName = "objectTest.png";

  std::string targetImage = TEST_IMAGE_OUTPUT_DIR;
  targetImage += "/" + imageName;
  Spire::GlobalTestEnvironment::instance()->writeFBO(targetImage);

  EXPECT_TRUE(Spire::fileExists(targetImage)) << "Failed to write output image! " << targetImage;

#ifdef TEST_PERCEPTUAL_COMPARE
  // Perform the perceptual comparison using the given regression directory.
  std::string compImage = TEST_IMAGE_COMPARE_DIR;
  compImage += "/" + imageName;

  ASSERT_TRUE(Spire::fileExists(compImage)) << "Failed to find comparison image! " << compImage;
  // Test using perceptula comparison program that the user has provided
  // (hopefully).
  std::string command = TEST_PERCEPTUAL_COMPARE_BINARY;
  command += " -threshold 50 ";
  command += targetImage + " " + compImage;

  // Usually the return code of std::system is implementation specific. But the
  // majority of systems end up returning the exit code of the program.
  if (std::system(command.c_str()) != 0)
  {
    // The images are NOT the same. Alert the user.
    FAIL() << "Perceptual compare of " << imageName << " failed.";
  }
#endif

#endif

}


//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestRenderingWithAttributes)
{
  // This test demonstrates a quick and dirty renderer usin
  // attributes and lambdas. Spire knows nothing about the objects, but allows
  // sufficient flexibility that it is possible to do many things.

  // First things first: just get the rendered image onto the filesystem...
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  std::fstream sphereFile("Assets/Sphere.sp");
  Interface::loadProprietarySR5AssetFile(sphereFile, *rawVBO, *rawIBO);

  std::vector<std::string> attribNames = {"aPos", "aNormal"};
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // Add necessary VBO's and IBO's
  std::string vboName = "vbo1";
  std::string iboName = "ibo1";
  mSpire->addVBO(vboName, rawVBO, attribNames);
  mSpire->addIBO(iboName, rawIBO, iboType);

  // Build shaders
  std::string shaderName = "DirGouraud";
  mSpire->addPersistentShader(
      shaderName, 
      { std::make_tuple("DirGouraud.vsh", Interface::VERTEX_SHADER), 
        std::make_tuple("DirGouraud.fsh", Interface::FRAGMENT_SHADER),
      });

  // Add object
  std::string objectName = "obj1";
  mSpire->addObject(objectName);
  mSpire->addPassToObject(objectName, shaderName, vboName, iboName, 
                          Interface::TRIANGLE_STRIP);
  mSpire->addLambdaObjectUniforms(objectName, lambdaUniformObjTrafs);
  
  // Object pass uniforms (can be set at a global level)
  mSpire->addObjectPassUniform(objectName, "uAmbientColor", V4(0.1f, 0.1f, 0.1f, 1.0f));
  mSpire->addObjectPassUniform(objectName, "uDiffuseColor", V4(0.8f, 0.8f, 0.0f, 1.0f));
  mSpire->addObjectPassUniform(objectName, "uSpecularColor", V4(0.5f, 0.5f, 0.5f, 1.0f));
  mSpire->addObjectPassUniform(objectName, "uSpecularPower", 32.0f);

  // Object spire attributes (used for computing appropriate uniforms).
  M44 xform;
  xform[3] = V4(1.0f, 0.0f, 0.0f, 1.0f);
  mSpire->addObjectPassMetadata(
      objectName, std::get<0>(TestCommonAttributes::getObjectToWorldTrafo()), xform);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(iboName);
  mSpire->removeVBO(vboName);

  // Global uniforms
  mSpire->addGlobalUniform("uLightDirWorld", V3(1.0f, 0.0f, 0.0f));

  // Setup camera uniforms.
  mCamera->setCommonUniforms(mSpire);

  mSpire->ntsDoFrame();

  // Write the resultant png to a temporary directory and compare against
  // the golden image results.
  /// \todo Look into using boost filesystem (but it isn't header-only). 

#ifdef TEST_OUTPUT_IMAGES
  std::string imageName = "attributeTest.png";

  std::string targetImage = TEST_IMAGE_OUTPUT_DIR;
  targetImage += "/" + imageName;
  Spire::GlobalTestEnvironment::instance()->writeFBO(targetImage);

  EXPECT_TRUE(Spire::fileExists(targetImage)) << "Failed to write output image! " << targetImage;

#ifdef TEST_PERCEPTUAL_COMPARE
  // Perform the perceptual comparison using the given regression directory.
  std::string compImage = TEST_IMAGE_COMPARE_DIR;
  compImage += "/" + imageName;

  ASSERT_TRUE(Spire::fileExists(compImage)) << "Failed to find comparison image! " << compImage;
  // Test using perceptula comparison program that the user has provided
  // (hopefully).
  std::string command = TEST_PERCEPTUAL_COMPARE_BINARY;
  command += " -threshold 2000 "; // 0.7% of a 640 x 480 image.
  command += targetImage + " " + compImage;

  // Usually the return code of std::system is implementation specific. But the
  // majority of systems end up returning the exit code of the program.
  if (std::system(command.c_str()) != 0)
  {
    // The images are NOT the same. Alert the user.
    FAIL() << "Perceptual compare of " << imageName << " failed.";
  }
#endif

#endif

}

//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestRenderingWithOutOfOrderAttributes)
{
  std::string testFileName = "orderOfAttributes.png";
  // Test the rendering of a phong shaded quad with out of order attributes.
  // aFieldData is unused in the phong shader.

  // Ensure shader is loaded.
  std::string shader1 = "DirPhong";
  mSpire->addPersistentShader(
      shader1, 
      { std::make_tuple("DirPhong.vsh", Spire::Interface::VERTEX_SHADER), 
        std::make_tuple("DirPhong.fsh", Spire::Interface::FRAGMENT_SHADER),
      });

  // Setup VBO / IBO.
  std::vector<float> vboData = 
  {
    -1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     1.0f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     1.0f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f, 1.0f
  };
  std::vector<std::string> attribNames = {"aPos", "aFieldData", "aNormal"};

  std::vector<uint16_t> iboData =
  {
    0, 1, 2, 3
  };
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  uint8_t*  rawBegin;
  size_t    rawSize;

  // Copy vboData into vector of uint8_t. Using std::copy.
  std::shared_ptr<std::vector<uint8_t>> rawVBO(new std::vector<uint8_t>());
  rawSize = vboData.size() * (sizeof(float) / sizeof(uint8_t));
  rawVBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&vboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawVBO->assign(rawBegin, rawBegin + rawSize);

  // Copy iboData into vector of uint8_t. Using std::vector::assign.
  std::shared_ptr<std::vector<uint8_t>> rawIBO(new std::vector<uint8_t>());
  rawSize = iboData.size() * (sizeof(uint16_t) / sizeof(uint8_t));
  rawIBO->reserve(rawSize);
  rawBegin = reinterpret_cast<uint8_t*>(&iboData[0]); // Remember, standard guarantees that vectors are contiguous in memory.
  rawIBO->assign(rawBegin, rawBegin + rawSize);

  // Add necessary VBO's and IBO's
  std::string vbo1 = "vbo1";
  std::string ibo1 = "ibo1";
  mSpire->addVBO(vbo1, rawVBO, attribNames);
  mSpire->addIBO(ibo1, rawIBO, iboType);

  // Setup object with default pass.
  std::string obj1 = "obj1";
  mSpire->addObject(obj1);
  
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP);
  mSpire->removeIBO(ibo1);
  mSpire->removeVBO(vbo1);

  mSpire->addObjectPassUniform(obj1, "uAmbientColor", V4(0.01f, 0.01f, 0.01f, 1.0f));
  mSpire->addObjectPassUniform(obj1, "uDiffuseColor", V4(0.0f, 0.8f, 0.0f, 1.0f));
  mSpire->addObjectPassUniform(obj1, "uSpecularColor", V4(0.0f, 0.0f, 0.0f, 1.0f));
  mSpire->addObjectPassUniform(obj1, "uSpecularPower", 16.0f);
  mSpire->addLambdaObjectUniforms(obj1, lambdaUniformObjTrafs);

  M44 xform;
  xform[3] = V4(0.0f, 0.0f, 0.0f, 1.0f);
  mSpire->addObjectPassMetadata(
      obj1, std::get<0>(TestCommonAttributes::getObjectToWorldTrafo()), xform);

  // Setup uniforms unrelated to our object.
  mSpire->addGlobalUniform("uLightDirWorld", V3(0.0f, 0.0f, 1.0f));
  mCamera->setCommonUniforms(mSpire);

  mSpire->ntsDoFrame();

  // Write the resultant png to a temporary directory and compare against
  // the golden image results.
  /// \todo Look into using boost filesystem (but it isn't header-only). 

#ifdef TEST_OUTPUT_IMAGES
  std::string targetImage = TEST_IMAGE_OUTPUT_DIR;
  targetImage += "/" + testFileName ;
  Spire::GlobalTestEnvironment::instance()->writeFBO(targetImage);

  EXPECT_TRUE(Spire::fileExists(targetImage)) << "Failed to write output image! " << targetImage;

#ifdef TEST_PERCEPTUAL_COMPARE
  // Perform the perceptual comparison using the given regression directory.
  std::string compImage = TEST_IMAGE_COMPARE_DIR;
  compImage += "/" + testFileName ;

  ASSERT_TRUE(Spire::fileExists(compImage)) << "Failed to find comparison image! " << compImage;
  // Test using perceptula comparison program that the user has provided
  // (hopefully).
  std::string command = TEST_PERCEPTUAL_COMPARE_BINARY;
  command += " -threshold 50 ";
  command += targetImage + " " + compImage;

  // Usually the return code of std::system is implementation specific. But the
  // majority of systems end up returning the exit code of the program.
  if (std::system(command.c_str()) != 0)
  {
    // The images are NOT the same. Alert the user.
    FAIL() << "Perceptual compare of " << testFileName << " failed.";
  }
#endif

#endif
}

}


