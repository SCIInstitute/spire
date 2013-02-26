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
#include "AppSpecific/SCIRun/SRInterface.h"
#include "StuPipe/StuInterface.h"

//------------------------------------------------------------------------------
// Basic StuPipe test fixture.
//------------------------------------------------------------------------------
class StuPipeTestFixture : public testing::Test
{
public:
  StuPipeTestFixture() {}

  virtual void SetUp() override
  {
    std::vector<std::string> shaderSearchDirs;
    shaderSearchDirs.push_back("Shaders");

    // Build StuPipe using the context from GlobalTestEnvironment.
    std::shared_ptr<Spire::Context> ctx = Spire::GlobalTestEnvironment::instance()->getContext();
    mSpireInterface = std::shared_ptr<Spire::Interface>(new Spire::Interface(
        ctx, shaderSearchDirs, false));

    // Build and bind StuPipe.
    mStuInterface = std::shared_ptr<Spire::StuInterface>(
        new Spire::StuInterface(mSpireInterface));
    mSpireInterface->pipePushBack(mStuInterface);
  }

  virtual void TearDown() override
  {
    mSpireInterface.reset();
  }

  std::shared_ptr<Spire::Interface>     mSpireInterface;
  std::shared_ptr<Spire::StuInterface>  mStuInterface;
};

//------------------------------------------------------------------------------
// SCIRun with StuPipe test fixture.
//------------------------------------------------------------------------------
class SCIRunStuPipeTestFixture : public testing::Test
{
public:
  SCIRunStuPipeTestFixture() {}

  virtual void SetUp() override
  {
    std::vector<std::string> shaderSearchDirs;
    shaderSearchDirs.push_back("Shaders");

    // Build StuPipe using the context from GlobalTestEnvironment.
    std::shared_ptr<Spire::Context> ctx = Spire::GlobalTestEnvironment::instance()->getContext();
    mSpireInterface = std::shared_ptr<Spire::SCIRun::SRInterface>(
        new Spire::SCIRun::SRInterface(ctx, shaderSearchDirs, false));

    // Build and bind StuPipe.
    mStuInterface = std::shared_ptr<Spire::StuInterface>(
        new Spire::StuInterface(mSpireInterface));
    mSpireInterface->pipePushBack(mStuInterface);
  }

  virtual void TearDown() override
  {
    mSpireInterface.reset();
  }

  std::shared_ptr<Spire::SCIRun::SRInterface> mSpireInterface;
  std::shared_ptr<Spire::StuInterface>        mStuInterface;
};

#endif 
