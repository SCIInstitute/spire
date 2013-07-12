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

#include "Core/Common.h"
#include "Core/Exceptions.h"
#include "Core/SpireObject.h"
#include "Core/FileUtil.h"
#include "AppSpecific/SCIRun/SRCommonUniforms.h"
#include "AppSpecific/SCIRun/SRCommonAttributes.h"

#include "GlobalTestEnvironment.h"
#include "CommonTestFixtures.h"

using namespace Spire;

namespace {

// Simple function to handle object transformations so that the GPU does not
// need to do the same calculation for each vertex.
static void lambdaUniformObjTrafs(ObjectLambdaInterface& iface, 
                                  std::list<Interface::UnsatisfiedUniform>& unsatisfiedUniforms)
{
  // Cache object to world transform.
  M44 objToWorld = iface.getObjectSpireAttribute<M44>(
      std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()));

  std::string objectTrafoName = std::get<0>(SRCommonUniforms::getObject());
  std::string objectToViewName = std::get<0>(SRCommonUniforms::getObjectToView());
  std::string objectToCamProjName = std::get<0>(SRCommonUniforms::getObjectToCameraToProjection());

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
          iface.getGlobalUniform<M44>(std::get<0>(SRCommonUniforms::getCameraToWorld())));
      LambdaInterface::setUniform<M44>(it->uniformType, it->uniformName,
                                       it->shaderLocation, inverseView * objToWorld);

      it = unsatisfiedUniforms.erase(it);
    }
    else if (it->uniformName == objectToCamProjName)
    {
      M44 inverseViewProjection = iface.getGlobalUniform<M44>(
          std::get<0>(SRCommonUniforms::getToCameraToProjection()));
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
TEST(InterfaceTests, TestSR5AssetLoader)
{
  std::ostringstream sRaw;

  // Write out the header.
  std::string header = "SCR5";
  sRaw.write(header.c_str(), 4);

  std::vector<V3> positions = 
  { V3(1.0f, 0.0f, 0.0f),
    V3(0.0f, 1.0f, 0.0f),
    V3(0.0f, 0.0f, 0.0f) };

  std::vector<V3> normals = 
  { V3(0.0f, 0.0f, 1.0f),
    V3(0.0f, 0.0f, 1.0f),
    V3(0.0f, 0.0f, 1.0f) };

  std::vector<uint16_t> indices = { 0,1,2 };

  // Helper functions for writing integers.
  auto writeUInt32 = [](std::ostream& ss, uint32_t i)
  { ss.write(reinterpret_cast<const char*>(&i), sizeof(uint32_t)); };

  auto writeUInt8 = [](std::ostream& ss, uint8_t i)
  { ss.write(reinterpret_cast<const char*>(&i), sizeof(uint8_t)); };

  // Number of meshes
  writeUInt32(sRaw, 1);

  // Number of vertices
  ASSERT_EQ(positions.size(), normals.size());
  writeUInt32(sRaw, positions.size());

  // Write out the positions / normals.
  size_t vec3Size = sizeof(float) * 3;
  for (int i = 0; i < positions.size(); i++)
  {
    V3 pos = positions[i];
    V3 norm = normals[i];
    sRaw.write(reinterpret_cast<const char*>(glm::value_ptr(pos)), vec3Size);
    sRaw.write(reinterpret_cast<const char*>(glm::value_ptr(norm)), vec3Size);
  }

  // Ensure we have triangles.
  ASSERT_EQ(0, indices.size() % 3);

  // Number of faces
  writeUInt32(sRaw, indices.size() / 3);
  for (int i = 0; i < indices.size(); i+=3)
  {
    writeUInt8(sRaw, 3);
    sRaw.write(reinterpret_cast<const char*>(&indices[i+0]), sizeof(uint16_t));
    sRaw.write(reinterpret_cast<const char*>(&indices[i+1]), sizeof(uint16_t));
    sRaw.write(reinterpret_cast<const char*>(&indices[i+2]), sizeof(uint16_t));
  }

  // Now read the stringstream in and grab the resultant vectors.
  std::istringstream ss(sRaw.str());
  std::vector<uint8_t> vbo;
  std::vector<uint8_t> ibo;
  size_t numTriangles = Interface::loadProprietarySR5AssetFile(ss, vbo, ibo);

  ASSERT_EQ(1, numTriangles);

  //std::istreambuf_iterator<uint8_t>(&vbo[0]);
  // Construct strings from vbo and ibo (perfectly valid, the strings, when
  // given a size, can contain the null character).
  std::string vboStr(reinterpret_cast<char*>(&vbo[0]), vbo.size());
  std::string iboStr(reinterpret_cast<char*>(&ibo[0]), ibo.size());
  std::istringstream vboStream(vboStr);
  std::istringstream iboStream(iboStr);

  // Darn it! I want polymorphic lambdas!
  // The following 2 anonymous functions can be collapsed down to one with
  // polymorphic lambdas.
  auto verifySSFloat = [](float expectedVal, std::istream& ss)
  {
    float fromStream;
    ss.read(reinterpret_cast<char*>(&fromStream), sizeof(float));
    ASSERT_FLOAT_EQ(expectedVal, fromStream);
  };

  auto verifySSUInt16 = [](uint16_t expectedVal, std::istream& ss)
  {
    uint16_t fromStream;
    ss.read(reinterpret_cast<char*>(&fromStream), sizeof(uint16_t));
    ASSERT_EQ(expectedVal, fromStream);
  };

  auto checkVector = [&verifySSFloat](const V3& vec, std::istream& ss)
  {
    verifySSFloat(vec.x, ss);
    verifySSFloat(vec.y, ss);
    verifySSFloat(vec.z, ss);
  };

  for (int i = 0; i < positions.size(); i++)
  {
    checkVector(positions[i], vboStream);
    checkVector(normals[i], vboStream);
  }

  for (uint16_t i : indices)
    verifySSUInt16(i, iboStream);
}

//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestPublicInterface)
{
  // This test is contrived and won't yield that much knowledge if you are 
  // attempting to learn the system.

  // REMEMBER:  We will always run the tests synchronously! So we will be able
  //            to catch errors immediately.

  std::string obj1 = "obj1";
  std::string obj2 = "obj2";
  std::string obj3 = "obj3";

  // We have a fresh instance of spire.
  mSpire->addObject(obj1);
  EXPECT_THROW(mSpire->addObject(obj1), Duplicate);
  EXPECT_EQ(1, mSpire->ntsGetNumObjects());

  // Add a new obj2.
  mSpire->addObject(obj2);
  EXPECT_THROW(mSpire->addObject(obj1), Duplicate);
  EXPECT_THROW(mSpire->addObject(obj2), Duplicate);
  EXPECT_EQ(2, mSpire->ntsGetNumObjects());

  // Remove and re-add object 1.
  mSpire->removeObject(obj1);
  EXPECT_EQ(1, mSpire->ntsGetNumObjects());
  mSpire->addObject(obj1);
  EXPECT_EQ(2, mSpire->ntsGetNumObjects());

  // Add a new obj3.
  mSpire->addObject(obj3);
  EXPECT_THROW(mSpire->addObject(obj1), Duplicate);
  EXPECT_THROW(mSpire->addObject(obj2), Duplicate);
  EXPECT_THROW(mSpire->addObject(obj3), Duplicate);
  EXPECT_EQ(3, mSpire->ntsGetNumObjects());
}

//------------------------------------------------------------------------------
TEST_F(InterfaceTestFixture, TestTriangle)
{
  // Test the rendering of a triangle.

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
  Interface::IBO_TYPE iboType = Interface::IBO_16BIT;

  // This is pretty contorted interface due to the marshalling between
  // std::vector<float> and std::vector<uint8_t>. In practice, you would want
  // to calculate the size of your VBO using one std::vector<uint8_t> and
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
      { {"UniformColor.vsh", Interface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Interface::FRAGMENT_SHADER},
      });

  // Test various cases of shader failure after adding a prior shader.
  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { {"UniformColor.vsh", Interface::FRAGMENT_SHADER}, 
        {"UniformColor.fsh", Interface::VERTEX_SHADER},
      }), std::invalid_argument);

  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { {"UniformColor2.vsh", Interface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Interface::FRAGMENT_SHADER},
      }), std::invalid_argument);

  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { {"UniformColor.vsh", Interface::VERTEX_SHADER}, 
        {"UniformColor2.fsh", Interface::FRAGMENT_SHADER},
      }), std::invalid_argument);

  // This final exception is throw directly from the addPersistentShader
  // function. The 3 prior exception were all thrown from the ShaderProgramMan.
  EXPECT_THROW(mSpire->addPersistentShader(
      shader1, 
      { {"UniformColor.vsh", Interface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Interface::FRAGMENT_SHADER},
      }), Duplicate);

  // Now construct passes (taking into account VBO attributes).

  // There exists no 'test obj'.
  EXPECT_THROW(mSpire->addPassToObject(
          "test obj", "UniformColor", "vbo", "ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Not a valid shader.
  EXPECT_THROW(mSpire->addPassToObject(
          obj1, "Bad Shader", "vbo", "ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Non-existant vbo.
  EXPECT_THROW(mSpire->addPassToObject(
          obj1, "UniformColor", "Bad vbo", "ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Non-existant ibo.
  EXPECT_THROW(mSpire->addPassToObject(
          obj1, "UniformColor", vbo1, "bad ibo",
          Interface::TRIANGLES),
      std::out_of_range);

  // Build a good pass.
  std::string pass1 = "pass1";

  // Attempt to add a pass to the object without the pass being present in the
  // system.
  EXPECT_THROW(mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1),
               std::runtime_error);

  // Add the pass to the system.
  mSpire->addPassToBack(pass1);

  // Now add the object pass. This automatically adds the object to the pass for
  // us. But the ordering within the pass is still arbitrary.
  mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1);

  // Attempt to re-add the good pass.
  EXPECT_THROW(mSpire->addPassToObject(obj1, shader1, vbo1, ibo1, Interface::TRIANGLE_STRIP, pass1),
               Duplicate);

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
  EXPECT_THROW(mSpire->addGlobalUniform("uProjIVObject", V3(0.0f, 0.0f, 0.0f)), ShaderUniformTypeError);

  // Add color to the pass (which will lookup the type via the shader).
  EXPECT_THROW(mSpire->addObjectPassUniform(obj1, "uColor", V3(0.0f, 0.0f, 0.0f), pass1), ShaderUniformTypeError);
  EXPECT_THROW(mSpire->addObjectPassUniform(obj1, "uColor", M44(), pass1), ShaderUniformTypeError);
  mSpire->addObjectPassUniform(obj1, "uColor", V4(1.0f, 0.0f, 0.0f, 1.0f), pass1);

  mSpire->ntsDoFrame();

  // Write the resultant png to a temporary directory and compare against
  // the golden image results.
  /// \todo Look into using boost filesystem (but it isn't header-only). 

#ifdef TEST_OUTPUT_IMAGES
  std::string imageName = "stuTriangle.png";

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

  // Attempt to set global uniform value that is at odds with information found
  // in the uniform manager (should induce a type error).

  /// \todo Test adding a uniform to the global state which does not have a
  ///       corresponding entry in the UniformManager.

  /// \todo Test uniforms.
  ///       1 - No uniforms set: should attempt to access global uniform state
  ///           manager and extract the uniform resulting in a std::out_of_range.
  ///       2 - Partial uniforms. Result same as #1.
  ///       3 - Uniform type checking. Ensure the types pulled from OpenGL
  ///           compiler matches our expected types.


  // Create an image of appropriate dimensions.

  /// \todo Test pass order using hasPassRenderingOrder on the object.
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
      { {"UniformColor.vsh", Interface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Interface::FRAGMENT_SHADER},
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
  mSpire->addObjectGlobalSpireAttribute<M44>(obj1, "objectTransform", testUniform);
  mSpire->addObjectPassSpireAttribute<M44>(obj1, "passTransform", testUniform, pass1);

  auto testMatrixEquality = [](const M44& a, const M44& b) {
    for (int c = 0; c < 4; c++)
    {
      for (int r = 0; r < 4; r++)
      {
        EXPECT_FLOAT_EQ(a[c][r], b[c][r]);
      }
    }
  };

  M44 retUnif;
  std::shared_ptr<const AbstractUniformStateItem> uniformItem;

  retUnif = object1->getObjectGlobalSpireAttribute("objectTransform")->getData<M44>();
  testMatrixEquality(retUnif, testUniform);

  EXPECT_THROW(object1->getObjectGlobalSpireAttribute("nonexistant"), std::runtime_error);

  uniformItem = object1->getObjectPassSpireAttribute(pass1, "passTransform");
  retUnif = uniformItem->getData<M44>();
  testMatrixEquality(retUnif, testUniform);

  uniformItem = object1->getObjectPassSpireAttribute(pass1, "nonexistant");
  EXPECT_EQ(nullptr, uniformItem);

  uniformItem = object1->getObjectPassSpireAttribute(SPIRE_DEFAULT_PASS, "objectTransform");
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
      { {"UniformColor.vsh", Interface::VERTEX_SHADER}, 
        {"UniformColor.fsh", Interface::FRAGMENT_SHADER},
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
      { {"DirGouraud.vsh", Interface::VERTEX_SHADER}, 
        {"DirGouraud.fsh", Interface::FRAGMENT_SHADER},
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
  mSpire->addObjectPassSpireAttribute(
      objectName, std::get<0>(SRCommonAttributes::getObjectToWorldTrafo()), xform);

  // No longer need VBO and IBO (will stay resident in the passes -- when the
  // passes are destroyed, the VBO / IBOs will be destroyed).
  mSpire->removeIBO(iboName);
  mSpire->removeVBO(vboName);

  // Global uniforms
  mSpire->addGlobalUniform("uLightDirWorld", V3(1.0f, 0.0f, 0.0f));

  // Setup camera uniforms.
  mCamera->setSRCommonUniforms(mSpire);

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

}

