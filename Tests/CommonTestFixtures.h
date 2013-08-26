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
/// \brief  Fixtures that are commonly used by the gtests.
///         No fixtures should use threading. They should all be synchronous.

#ifndef SPIRE_TESTS_COMMONTESTFIXTURES_H
#define SPIRE_TESTS_COMMONTESTFIXTURES_H

#include "gtest/gtest.h"
#include "GlobalTestEnvironment.h"
#include "SpireExt/SCIRun/SRInterface.h"

#include "TestCamera.h"

//------------------------------------------------------------------------------
// Basic interface test fixture.
//------------------------------------------------------------------------------
class InterfaceTestFixture : public testing::Test
{
public:
  InterfaceTestFixture () {}

  virtual void SetUp() override
  {
    std::vector<std::string> shaderSearchDirs;
    shaderSearchDirs.push_back("Shaders");

    // Build spire using the context from GlobalTestEnvironment.
    std::shared_ptr<Spire::Context> ctx = Spire::GlobalTestEnvironment::instance()->getContext();
    ctx->makeCurrent();
    mSpire = std::shared_ptr<Spire::Interface>(new Spire::Interface(
        ctx, shaderSearchDirs, false));

    // Add default attributes.
    addDefaultAttributes();

    // Build camera that we will use for testing purposes.
    mCamera = std::unique_ptr<TestCamera>(new TestCamera);
  }

  virtual void TearDown() override
  {
    mSpire.reset();
  }

  void addDefaultAttributes()
  {
    mSpire->addShaderAttribute("aPos",         3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
    mSpire->addShaderAttribute("aNormal",      3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
    mSpire->addShaderAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  Spire::Interface::TYPE_FLOAT);
    mSpire->addShaderAttribute("aColor",       4,  true,   sizeof(char) * 4,   Spire::Interface::TYPE_UBYTE);

    // Used to test attribute mis-alignment.
    mSpire->addShaderAttribute("aFieldData",   1,  false,  sizeof(float) * 1,  Spire::Interface::TYPE_FLOAT);
  }

  std::shared_ptr<Spire::Interface>     mSpire;
  std::unique_ptr<TestCamera>           mCamera;
};

//------------------------------------------------------------------------------
// SCIRun test fixture.
//------------------------------------------------------------------------------
class SCIRunTestFixture : public testing::Test
{
public:
  SCIRunTestFixture() {}

  virtual void SetUp() override
  {
    std::vector<std::string> shaderSearchDirs;
    shaderSearchDirs.push_back("Shaders");

    // Build spire using the context from GlobalTestEnvironment.
    std::shared_ptr<Spire::Context> ctx = Spire::GlobalTestEnvironment::instance()->getContext();
    mSpire = std::shared_ptr<Spire::SCIRun::SRInterface>(
        new Spire::SCIRun::SRInterface(ctx, shaderSearchDirs, false));

    // Add default attributes.
    addDefaultAttributes();

    // Build camera that we will use for testing purposes.
    mCamera = std::unique_ptr<TestCamera>(new TestCamera);
  }

  virtual void TearDown() override
  {
    mSpire.reset();
  }

  void addDefaultAttributes()
  {
    mSpire->addShaderAttribute("aPos",         3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
    mSpire->addShaderAttribute("aNormal",      3,  false,  sizeof(float) * 3,  Spire::Interface::TYPE_FLOAT);
    mSpire->addShaderAttribute("aColorFloat",  4,  false,  sizeof(float) * 4,  Spire::Interface::TYPE_FLOAT);
    mSpire->addShaderAttribute("aColor",       4,  true,   sizeof(char) * 4,   Spire::Interface::TYPE_UBYTE);
  }

  std::shared_ptr<Spire::SCIRun::SRInterface> mSpire;
  std::unique_ptr<TestCamera>                 mCamera;
};

#endif 
