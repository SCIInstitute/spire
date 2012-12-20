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
#include "High/MurmurHash3.h"
#include "High/ShaderAttributeMan.h"

using namespace Spire;

namespace {

//------------------------------------------------------------------------------
TEST(ShaderAttributeManBasic, TestUnknownAttribute)
{
  ShaderAttributeMan attribMan(false);

  ASSERT_EQ(1, attribMan.getNumAttributes());

  std::string unknownName = ShaderAttributeMan::UNKNOWN_NAME;
  AttribState state;

  // Test function return values.
  EXPECT_NO_THROW(state = attribMan.getAttributeAtIndex(
          ShaderAttributeMan::UNKNOWN_ATTRIBUTE_INDEX));
  EXPECT_EQ(true, std::get<0>(attribMan.findAttributeWithName(unknownName)));
  EXPECT_EQ(0, std::get<1>(attribMan.findAttributeWithName(unknownName)));

  // Test string hashing.
  uint32_t hashOut = 0;
  MurmurHash3_x86_32(
      static_cast<const void*>(unknownName.c_str()),
      static_cast<int>(unknownName.size()),
      ShaderAttributeMan::MURMUR_SEED_VALUE,
      static_cast<void*>(&hashOut));
  EXPECT_EQ(hashOut, state.nameHash);
  EXPECT_EQ(hashOut, attribMan.hashString(unknownName));

  // Test returned attribMan.
  EXPECT_EQ(0, state.index);
}

//------------------------------------------------------------------------------
TEST(ShaderAttributeManBasic, TestUnknownExceptions)
{
  ShaderAttributeMan attribMan(false);

  // Attempt to access the element passed the end of the array.
  EXPECT_THROW(
      attribMan.getAttributeAtIndex(1),
      std::range_error);

  std::string bogusName = "someName";
  EXPECT_EQ(false, std::get<0>(attribMan.findAttributeWithName(bogusName)));
  EXPECT_EQ(0, std::get<1>(attribMan.findAttributeWithName(bogusName)));
}

//------------------------------------------------------------------------------
TEST(ShaderAttributeManBasic, TestDefaultAttributes)
{
  
}

//------------------------------------------------------------------------------
class ShaderAttributeManInvolved : public testing::Test
{
protected:

  virtual void SetUp()
  {
    
  }

  ShaderAttributeMan  mAttribMan;
};

//------------------------------------------------------------------------------
TEST_F(ShaderAttributeManInvolved, findingAttributes)
{
  // When using a test fixture, it is the same as deriving from the fixture class.
  
}

}
