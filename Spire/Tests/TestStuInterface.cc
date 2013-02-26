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

#include <gtest/gtest.h>
#include "Common.h"
#include "Exceptions.h"
#include "StuPipe/StuInterface.h"
#include "GlobalTestEnvironment.h"
#include "CommonTestFixtures.h"

using namespace Spire;

namespace {

/// \todo Need an option to run OpenGL through a mocked up interface. This
///       would also require mocking some GLEW functions as well.
///       Or just have compiler switches that remove any OpenGL calls.
///       Need to have this support to test beyond the basics.
///       Possibly the best solution to all of this is to just create a valid
///       OpenGL context when testing Spire as well. In this manner, I can
///       delete / create Spire objects at will.

//------------------------------------------------------------------------------
TEST_F(StuPipeTestFixture, TestObjectCreation)
{
  // We have a fresh instance of spire with a StuPipe bound.


  /// \todo Create a test fixture that creates a spire instance that we can use
  ///       for testing purposes.

  ///// Assuming StuInterface is already created somehow...
  //StuInterface interface;

  //interface.addObject("obj1");
  //EXPECT_THROW(interface.addObject("obj1"), Duplicate);
}

}

