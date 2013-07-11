/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2012 Scientific Computing and Imaging Institute,
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
/// \date   December 2012

#include <gtest/gtest.h>
#include "Core/Common.h"
#include "Core/Exceptions.h"
#include "Core/ShaderUniformMan.h"

using namespace Spire;

namespace {

//------------------------------------------------------------------------------
TEST(ShaderUniformManBasic, TestUnknownUniform)
{
  ShaderUniformMan uniformMan;
  ASSERT_EQ(1, uniformMan.getNumUniforms());

  // Test unknown name (the 1 uniform initially placed in the uniform man).
  std::string unknownName = ShaderUniformMan::getUnknownName();
  EXPECT_NO_THROW(uniformMan.getUniformWithName(unknownName));

  std::string bogusName = "someName";
  EXPECT_THROW(uniformMan.getUniformWithName(bogusName), std::out_of_range);
}

//------------------------------------------------------------------------------
class ShaderUniformManInvolved : public testing::Test
{
protected:
  ShaderUniformManInvolved() :
      mUniformMan()
  {}

  virtual void SetUp()    {}
  virtual void TearDown() {}

  ShaderUniformMan  mUniformMan;
};

//------------------------------------------------------------------------------
TEST_F(ShaderUniformManInvolved, addingUniforms)
{
  // Add uniforms and ensure that they have been appropriately added.
  std::string uniformName = "uniform1";
  mUniformMan.addUniform(uniformName, GL_FLOAT_MAT4);

  // This test uses a different method of obtaining the uniform than the
  // next uniform. This uses findUniformWithName, while the other uses
  // getUniformWithName and expects that no exceptions are thrown.
  std::shared_ptr<const UniformState> state;
  ASSERT_NO_THROW(state = mUniformMan.getUniformWithName(uniformName));
  EXPECT_EQ(uniformName, state->codeName);
  EXPECT_EQ(GL_FLOAT_MAT4, state->type);

  uniformName = "uniform2";
  mUniformMan.addUniform(uniformName, GL_FLOAT_VEC4);
  ASSERT_NO_THROW(state = mUniformMan.getUniformWithName(uniformName));
  EXPECT_EQ(uniformName, state->codeName);
  EXPECT_EQ(GL_FLOAT_VEC4, state->type);
}

}
