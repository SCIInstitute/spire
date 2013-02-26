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

  // We have a fresh instance of spire with a StuPipe bound.
  EXPECT_EQ(0, mStuInterface->ntsGetRenderOrder());
  mStuInterface->addObject("obj1");
  EXPECT_THROW(mStuInterface->addObject("obj1"), Duplicate);
  EXPECT_EQ(1, mStuInterface->ntsGetNumObjects());

  // Add a new obj2.
  mStuInterface->addObject("obj2");
  EXPECT_THROW(mStuInterface->addObject("obj1"), Duplicate);
  EXPECT_THROW(mStuInterface->addObject("obj2"), Duplicate);
  EXPECT_EQ(2, mStuInterface->ntsGetNumObjects());

  // Remove and re-add object 1.
  mStuInterface->removeObject("obj1");
  EXPECT_EQ(1, mStuInterface->ntsGetNumObjects());
  mStuInterface->addObject("obj1");
  EXPECT_EQ(2, mStuInterface->ntsGetNumObjects());

  // Add a new obj3.
  mStuInterface->addObject("obj3");
  EXPECT_THROW(mStuInterface->addObject("obj1"), Duplicate);
  EXPECT_THROW(mStuInterface->addObject("obj2"), Duplicate);
  EXPECT_THROW(mStuInterface->addObject("obj3"), Duplicate);
  EXPECT_EQ(3, mStuInterface->ntsGetNumObjects());


  // Verify rendering orders.

  // Re-assign rendering orders for objects.
}

//------------------------------------------------------------------------------
TEST_F(StuPipeTestFixture, TestTriangle)
{
  // Test the rendering of a triangle with StuPipe.
}

//------------------------------------------------------------------------------
TEST_F(StuPipeTestFixture, TestCube)
{
  // Test the rendering of a cube with the StuPipe
}

}

