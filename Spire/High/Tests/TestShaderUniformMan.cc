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
#include "Common.h"
#include "Exceptions.h"
#include "High/MurmurHash3.h"
#include "High/ShaderUniformMan.h"

using namespace Spire;

namespace {

//------------------------------------------------------------------------------
TEST(ShaderUniformManBasic, TestUnknownUniform)
{
  ShaderUniformMan uniformMan(false);

  ASSERT_EQ(1, uniformMan.getNumUniforms());

  std::string unknownName = ShaderUniformMan::getUnknownName();
  UniformState state;

  // Test function return values.
  EXPECT_NO_THROW(state = uniformMan.getUniformAtIndex(
          ShaderUniformMan::getUnknownUniformIndex()));
  EXPECT_EQ(true, std::get<0>(uniformMan.findUniformWithName(unknownName)));
  EXPECT_EQ(0, std::get<1>(uniformMan.findUniformWithName(unknownName)));

  // Test string hashing.
  uint32_t hashOut = 0;
  MurmurHash3_x86_32(
      static_cast<const void*>(unknownName.c_str()),
      static_cast<int>(unknownName.size()),
      ShaderUniformMan::getMurmurSeedValue(),
      static_cast<void*>(&hashOut));
  EXPECT_EQ(hashOut, state.nameHash);
  EXPECT_EQ(hashOut, uniformMan.hashString(unknownName));

  // Test returned uniformMan.
  EXPECT_EQ(0, state.index);
}

//------------------------------------------------------------------------------
TEST(ShaderUniformManBasic, TestUnknownExceptions)
{
  ShaderUniformMan uniformMan(false);

  // Attempt to access the element passed the end of the array.
  EXPECT_THROW(
      uniformMan.getUniformAtIndex(1),
      std::range_error);

  std::string bogusName = "someName";
  EXPECT_THROW(uniformMan.getUniformWithName(bogusName), NotFound);

  EXPECT_EQ(false, std::get<0>(uniformMan.findUniformWithName(bogusName)));
  EXPECT_EQ(0, std::get<1>(uniformMan.findUniformWithName(bogusName)));
}

//------------------------------------------------------------------------------
TEST(ShaderUniformManBasic, TestDefaultUniform)
{
  ShaderUniformMan uniformMan(true);

  EXPECT_LE(1, uniformMan.getNumUniforms());
}

//------------------------------------------------------------------------------
class ShaderUniformManInvolved : public testing::Test
{
protected:
  ShaderUniformManInvolved() :
      mUniformMan(true)
  {}

  virtual void SetUp()    {}
  virtual void TearDown() {}

  ShaderUniformMan  mUniformMan;
};

//------------------------------------------------------------------------------
TEST_F(ShaderUniformManInvolved, addingUniforms)
{
  // When using a test fixture it is the same as deriving from the fixture class.
  size_t beginSize = mUniformMan.getNumUniforms();

  // Add uniforms and ensure that they have been appropriately added.
  std::string uniformName = "uniform1";
  mUniformMan.addUniform(uniformName);

  // This test uses a different method of obtaining the uniform than the
  // next uniform. This uses findUniformWithName, while the other uses
  // getUniformWithName and expects that no exceptions are thrown.
  UniformState state;
  std::tuple<bool, size_t> uniformIndex = mUniformMan.findUniformWithName(uniformName);
  ASSERT_EQ(true, std::get<0>(uniformIndex));
  state = mUniformMan.getUniformAtIndex(std::get<1>(uniformIndex));
  EXPECT_EQ(beginSize, state.index);
  EXPECT_EQ(uniformName, state.codeName);
  EXPECT_EQ(mUniformMan.hashString(uniformName), state.nameHash);


  uniformName = "uniform2";
  mUniformMan.addUniform(uniformName);
  ASSERT_NO_THROW(state = mUniformMan.getUniformWithName(uniformName));
  EXPECT_EQ(beginSize + 1, state.index);
  EXPECT_EQ(uniformName, state.codeName);
  EXPECT_EQ(mUniformMan.hashString(uniformName), state.nameHash);

  EXPECT_EQ(beginSize + 2, mUniformMan.getNumUniforms());
}

//------------------------------------------------------------------------------
class ShaderUniformCollectionTests : public testing::Test
{
protected:
  ShaderUniformCollectionTests() :
      mUniformMan(true),
      mCol1(mUniformMan, 0),
      mCol2(mUniformMan, 0),
      mCol3(mUniformMan, 0)
  {}

  virtual void SetUp()
  {
    mUniformMan.addUniform("at1");
    mUniformMan.addUniform("at2");
    mUniformMan.addUniform("at3");
    mUniformMan.addUniform("at4");

    mCol1.addUniform("at1");
    mCol1.addUniform("at3");

    /// The following are purposefully out-of-order
    mCol2.addUniform("at1");
    mCol2.addUniform("at3");
    mCol2.addUniform("at2");

    mCol3.addUniform("at4");
    mCol3.addUniform("at1");
    mCol3.addUniform("at3");
    mCol3.addUniform("at2");
  }
  virtual void TearDown() {}

  ShaderUniformMan            mUniformMan;
  ShaderUniformCollection   mCol1;
  ShaderUniformCollection   mCol2;
  ShaderUniformCollection   mCol3;
};

//------------------------------------------------------------------------------
TEST_F(ShaderUniformCollectionTests, primaryTest)
{
  UniformState state;

  // Check first uniform and most of its values.
  ASSERT_NO_THROW(state = mCol1.getUniform(0).uniform);
  EXPECT_EQ("at1", state.codeName);

  // Now just check that the uniforms are in the expected order.
  ASSERT_NO_THROW(state = mCol1.getUniform(1).uniform);
  EXPECT_EQ("at3", state.codeName);

  ASSERT_NO_THROW(state = mCol2.getUniform(0).uniform);
  EXPECT_EQ("at1", state.codeName);
  ASSERT_NO_THROW(state = mCol2.getUniform(1).uniform);
  EXPECT_EQ("at2", state.codeName);
  ASSERT_NO_THROW(state = mCol2.getUniform(2).uniform);
  EXPECT_EQ("at3", state.codeName);

  ASSERT_NO_THROW(state = mCol3.getUniform(0).uniform);
  EXPECT_EQ("at1", state.codeName);
  ASSERT_NO_THROW(state = mCol3.getUniform(1).uniform);
  EXPECT_EQ("at2", state.codeName);
  ASSERT_NO_THROW(state = mCol3.getUniform(2).uniform);
  EXPECT_EQ("at3", state.codeName);
  ASSERT_NO_THROW(state = mCol3.getUniform(3).uniform);
  EXPECT_EQ("at4", state.codeName);
}

}
